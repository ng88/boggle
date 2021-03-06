#include <unistd.h>

#include "assert.h"
#include "boggle.h"

#ifdef G_WINDOWS
# include "SDL.h"
# include "SDL_thread.h"
#else
# include "SDL/SDL.h"
# include "SDL/SDL_thread.h"
#endif

#include <agar/core.h>
#include <agar/gui.h>

#include "sdl_draw.h"

#define RES_BOXES "res/carre.bmp"
#define RES_FONT "res/font1.bmp"
#define RES_FONT2 "res/font2.bmp"
#define RES_SEL "res/selecteur.bmp"
#define RES_COLORS "res/color.acs"

#define BGCOLOR 0xffffff

/* emplacement du plateau */
#define BOARD_START_X 10
#define BOARD_START_Y 10

/* espace apres le plateau */
#define  BOARD_RIGHT 10
#define BOARD_BOTTOM (10+25)

/* taille d'une case */
#define BOX_SIZE_X 58
#define BOX_SIZE_Y 58

/* taille d'une lettre */
#define FONT_SIZE_X 29
#define FONT_SIZE_Y 25

#define MAIN_WIN_SIZE_X 640
#define MAIN_WIN_SIZE_Y 460

#define FOUND_WIN_SIZE_X 200
#define FOUND_WIN_SIZE_Y 260

#define SCORE_WIN_SIZE_X FOUND_WIN_SIZE_X
#define SCORE_WIN_SIZE_Y 100

#define CMD_WIN_SIZE_X FOUND_WIN_SIZE_X
#define CMD_WIN_SIZE_Y 80

#define NEW_WIN_SIZE_X 200
#define NEW_WIN_SIZE_Y 100

#define WAIT_WIN_SIZE_X 250
#define WAIT_WIN_SIZE_Y 50

#define NORMAL_FPS 30
#define MINIMAL_FPS 2

enum { GP_NOT_PLAYING = 0, GP_PLAYING = 1, GP_WATCHING_SOLUTION = 2 };

static volatile int stop;
static volatile int changed;
static volatile int playing;
static volatile int old_playing;
static SDL_Surface *screen;
static SDL_Surface *font;
static SDL_Surface *font2;
static SDL_Surface *boxes;
static board_t * current_board;
static AG_Window * win_new;
static AG_Window * win_wait;
static AG_Window * win_main;
static AG_Table * tbl_solution;
static AG_Notebook * tabs;
static AG_NotebookTab * tab_solution;
static AG_NotebookTab * tab_user;
static SDL_Thread * thread = NULL;
static int time_left_sec = 0;
static int time_left_min = 0;
static volatile int new_board_size = 4;
static volatile int new_time       = 3;

void render();
void init();
void do_events();
void show_solution();
int thread_create_game(void* d);

static int sort_string(const void *p1, const void *p2)
{
	const AG_TableCell *c1 = p1;
	const AG_TableCell *c2 = p2;
	return (strcoll(c1->data.s, c2->data.s));
}
 
static int sort_int(const void *p1, const void *p2)
{
	const AG_TableCell *c1 = p1;
	const AG_TableCell *c2 = p2;
	return (c1->data.i - c2->data.i);
}

static void update_foundtable(AG_Event *event)
{

    static size_t last_size = 0;

    if(last_size != vector_size(current_board->foundword))
    {
	last_size = vector_size(current_board->foundword);

	AG_Table * tbl = AG_SELF();
	AG_TableBegin(tbl);

	size_t i;
	for(i = 0; i < last_size; ++i)
	{
	    char * w = (char*)vector_get_element_at(current_board->foundword, i);
	    AG_TableAddRow(tbl, "%s:%d", w, boggle_score_for_word(w));
	}
	//AG_TableAddRow(tbl, "%s:%d", "Total:", current_board->score);

	AG_TableEnd(tbl);
    }
}


static void board_size_changed(AG_Event *event)
{
    AG_TlistItem * item = AG_PTR(1);
    new_board_size = (int)item->p1;
}

static void time_changed(AG_Event *event)
{
    AG_TlistItem * item = AG_PTR(1);
    new_time = (int)item->p1;
}

static void button_giveup(AG_Event *event)
{

    if(playing != GP_PLAYING)
    {
	AG_TextMsg(AG_MSG_ERROR, "No game in progress!");
	return;
    }

    playing = GP_WATCHING_SOLUTION;
    show_solution();
}

static void button_quit(AG_Event *event)
{
    AG_Quit();
}


static void button_new_valid(AG_Event *event)
{

    if(new_board_size == -1)
    {
	AG_TextMsg(AG_MSG_ERROR, "Please select a board size!");
	return;
    }


    AG_WindowHide(win_new);
    AG_WindowShow(win_wait);

    AG_NotebookSelectTab(tabs, tab_user);
    AG_TableBegin(tbl_solution);

    if(thread)
	SDL_WaitThread(thread, NULL);

    thread = SDL_CreateThread(thread_create_game, NULL);

}

static void button_new_cancel(AG_Event *event)
{
    AG_WindowHide(win_new);
    playing = old_playing;
    changed = 1;
}

static void button_new(AG_Event *event)
{
    old_playing = playing;
    playing = GP_NOT_PLAYING;
    AG_WindowShow(win_new);

}

static void tbl_solution_click(AG_Event *event)
{
    int i = AG_INT(1);
    boggle_highlight_fullword(current_board, 
			      (char*)vector_get_element_at(current_board->wordlist, i));
    changed = 1;
}

void show_solution()
{
    AG_TableBegin(tbl_solution);

    size_t i;
    for(i = 0; i < vector_size(current_board->wordlist); ++i)
    {
	char * w = (char*)vector_get_element_at(current_board->wordlist, i);
	AG_TableAddRow(tbl_solution, "%s:%d", w, boggle_score_for_word(w));
    }

    AG_TableEnd(tbl_solution);

    AG_NotebookSelectTab(tabs, tab_solution);
}

void timer(AG_Event *event)
{
    if(playing != GP_PLAYING || time_left_min == -1)
	return;

    if(time_left_sec == 0)
    {
	if(time_left_min == 0)
	{
	    AG_TextMsg(AG_MSG_INFO, "Time is up!");
	    playing = GP_WATCHING_SOLUTION;
	    show_solution();
	    return;
	}
	
	time_left_min--;
	time_left_sec = 59;
    }
    else
	time_left_sec--;

    AG_SchedEvent(win_main, win_main, 1000, "check_time", NULL);
}

int thread_create_game(void * d)
{
    AG_SetRefreshRate(MINIMAL_FPS);

    if(new_time)
    {
	time_left_min = new_time;
	time_left_sec = 0;
    }
    else
	time_left_min = time_left_sec = -1;

    boggle_resize_board(current_board, new_board_size);
    boggle_fill_board(current_board);
    boggle_create_wordlist(current_board);
    boggle_start_game(current_board);
    boggle_print_board(current_board);

    draw_rect(screen, 0, 0, screen->w, screen->h, BGCOLOR);

    AG_SetRefreshRate(NORMAL_FPS);

    AG_WindowHide(win_wait);

    changed = 1;
    playing = GP_PLAYING;

    if(time_left_min != -1)
	timer(NULL);

    return 0;
}


void boggle_start_ihm(board_t * b)
{

    c_assert(b);
    current_board = b;

    if (AG_InitCore("Super Boggle of the future", 0) == -1) {
	fprintf(stderr, "%s\n", AG_GetError());
	return;
    }
    if (AG_InitVideo(MAIN_WIN_SIZE_X, MAIN_WIN_SIZE_Y, 32, 
		     SDL_SWSURFACE | 
		     SDL_DOUBLEBUF
	    ) == -1)
    {
	fprintf(stderr, "%s\n", AG_GetError());
	return;
    }

    screen = agView->v;

    AG_ColorsLoad(RES_COLORS);

    AG_Window *win, *win_cmd, *win_score;

    win = AG_WindowNew(
	AG_WINDOW_NOTITLE |
	AG_WINDOW_NOBORDERS
	);
    AG_WindowSetGeometry(win, 
			 MAIN_WIN_SIZE_X - FOUND_WIN_SIZE_X, CMD_WIN_SIZE_Y,
			 FOUND_WIN_SIZE_X, FOUND_WIN_SIZE_Y);


    tabs = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
    tab_user = AG_NotebookAddTab(tabs, "Found word", AG_BOX_VERT);

    AG_Table * tbl = AG_TableNewPolled(tab_user, AG_TABLE_EXPAND, update_foundtable, NULL);
    AG_TableAddCol(tbl, "Word", "<LARGER WORD POSS>", &sort_string);
    AG_TableAddCol(tbl, "Score", NULL, &sort_int);

    tab_solution = AG_NotebookAddTab(tabs, "Solution", AG_BOX_VERT);
    tbl_solution = AG_TableNew(tab_solution, AG_TABLE_EXPAND);	
    AG_TableAddCol(tbl_solution, "Word", "<LARGER WORD POSS>", &sort_string);
    AG_TableAddCol(tbl_solution, "Score", NULL, &sort_int);
    AG_TableSetRowDblClickFn(tbl_solution, &tbl_solution_click, NULL);

    AG_WindowShow(win);
    win_main = win;

    AG_AddEvent(win_main, "check_time", timer, NULL);



    win_cmd = AG_WindowNew(AG_WINDOW_NOTITLE | AG_WINDOW_NOBORDERS);
    AG_WindowSetGeometry(win_cmd, 
			 MAIN_WIN_SIZE_X - FOUND_WIN_SIZE_X, 0,
			 CMD_WIN_SIZE_X, CMD_WIN_SIZE_Y);
    AG_VBox * vbox = AG_VBoxNew(win_cmd, AG_VBOX_HFILL | AG_BOX_HOMOGENOUS);

    AG_ButtonNewFn(vbox, 0, "New game", &button_new, NULL);
    AG_ButtonNewFn(vbox, 0, "Give up", &button_giveup, NULL);
    AG_ButtonNewFn(vbox, 0, "Quit", &button_quit, NULL);
    AG_WindowShow(win_cmd);





    win_score = AG_WindowNew(AG_WINDOW_NOTITLE | AG_WINDOW_NOBORDERS);
    AG_WindowSetGeometry(win_score, 
			 MAIN_WIN_SIZE_X - FOUND_WIN_SIZE_X, CMD_WIN_SIZE_Y + FOUND_WIN_SIZE_Y,
			 SCORE_WIN_SIZE_X, SCORE_WIN_SIZE_Y);
    AG_Label *lbl = AG_LabelNewPolled(win_score, 0, "Time: %d:%d\nScore: %u\nCompleted: %f%%\nFound word: %u\nPossible word: %u",
		      &time_left_min, &time_left_sec,
		      &b->score,
		      &b->scpercent,
		      &b->foundword->size,
		      &b->wordlist->size);
    AG_LabelSizeHint(lbl, 4,
		     "Time: BLABLA\nScore: 10000\nCompleted: 100%\nFound word: 10000\nPossible word: 10000");
    AG_WindowShow(win_score);






    AG_TlistItem * item;
   win_new = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_NOCLOSE);
   AG_WindowSetGeometry(win_new, 
			MAIN_WIN_SIZE_X / 2 - NEW_WIN_SIZE_X / 2,
			MAIN_WIN_SIZE_Y / 2 - NEW_WIN_SIZE_Y / 2,
			NEW_WIN_SIZE_X, NEW_WIN_SIZE_Y);
    AG_WindowSetCaption(win_new, "New game");

    AG_Combo * com = AG_ComboNew(win_new, AG_COMBO_HFILL, "Board size: ");
    AG_ComboSizeHint(com, "0 x 1", 6);
    AG_TlistSetChangedFn(com->list, board_size_changed, NULL);

    int i;
    for (i = 2; i <= 7; i++)
    {
	item = AG_TlistAdd(com->list, NULL, "%d x %d", i, i);
	item->p1 = (void*)i;
	if(i == new_board_size)
	    AG_ComboSelect(com, item);
    }


    com = AG_ComboNew(win_new, AG_COMBO_HFILL, "Time: ");
    AG_ComboSizeHint(com, "10 minutes", 11);
    AG_TlistSetChangedFn(com->list, time_changed, NULL);

    item = AG_TlistAdd(com->list, NULL, "unlimited", i, i);
    item->p1 = (void*)0;


    for (i = 1; i <= 10; i++)
    {
	item = AG_TlistAdd(com->list, NULL, "%d minutes", i, i);
	item->p1 = (void*)i;
	if(i == new_time)
	    AG_ComboSelect(com, item);
    }

    AG_SpacerNew(win_new, AG_SEPARATOR_VERT);

    AG_HBox * hbox = AG_HBoxNew(win_new, AG_HBOX_HFILL | AG_BOX_HOMOGENOUS);

    AG_SpacerNew(hbox, AG_SEPARATOR_HORIZ);
    AG_ButtonNewFn(hbox, 0, "OK", &button_new_valid, NULL);
    AG_ButtonNewFn(hbox, 0, "Cancel", &button_new_cancel, NULL);
    AG_SpacerNew(hbox, AG_SEPARATOR_HORIZ);





    win_wait = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_NOCLOSE | AG_WINDOW_NORESIZE);
    AG_WindowSetGeometry(win_wait, 
			MAIN_WIN_SIZE_X / 2 - WAIT_WIN_SIZE_X / 2,
			MAIN_WIN_SIZE_Y / 2 - WAIT_WIN_SIZE_Y / 2,
			WAIT_WIN_SIZE_X, WAIT_WIN_SIZE_Y);
    AG_WindowSetCaption(win_wait, "Information");
    AG_LabelJustify(AG_LabelNewString(win_wait, AG_LABEL_HFILL, "Please wait while the game is loading..."), AG_TEXT_CENTER);

    AG_SetRefreshRate(NORMAL_FPS);


    if(!screen)
    {
	fprintf(stderr, "Unable to set to correct video mode, %s\n", SDL_GetError());
	return;
    }

    //AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);

    init(b);
  
    playing = GP_NOT_PLAYING;
    changed = 1;

    extern struct ag_objectq agTimeoutObjQ;
    SDL_Event ev;
    Uint32 Tr1 = SDL_GetTicks(), Tr2 = 0;
 
    while(!stop)
    {
	if(playing)
	    render(b);

	Tr2 = SDL_GetTicks();

	if(Tr2-Tr1 >= agView->rNom)
	{		/* Time to redraw? */
	    AG_LockVFS(agView);

	    /*
	     * Render the GUI windows and generate a list of
	     * dirty rectangles.
	     */
	    AG_TAILQ_FOREACH(win, &agView->windows, windows)
	    {
		AG_ObjectLock(win);
		if(!win->visible)
		{
		    AG_ObjectUnlock(win);
		    continue;
		}

		AG_WidgetDraw(win);
		if(!(win->flags & AG_WINDOW_NOUPDATERECT))
		{
		    AG_QueueVideoUpdate(
			AGWIDGET(win)->x, AGWIDGET(win)->y,
			AGWIDGET(win)->w, AGWIDGET(win)->h);
		}
		AG_ObjectUnlock(win);
	    }

	    /*
	     * Update the dirty rectangles..
	     */
	    if (agView->ndirty > 0)
	    {

		SDL_UpdateRects(agView->v,
				agView->ndirty,
				agView->dirty);
		
		agView->ndirty = 0;
	    }
	    AG_UnlockVFS(agView);
 
	    /* Recalibrate the effective refresh rate. */
	    Tr1 = SDL_GetTicks();
	    agView->rCur = agView->rNom - (Tr1-Tr2);
	    if (agView->rCur < 1)
		agView->rCur = 1;
	    
	}
	else if (SDL_PollEvent(&ev) != 0)
	{
	    /* Send all SDL events to Agar-GUI. */

	    bool processed = false;
	    if(playing == GP_PLAYING)
	    {
		switch(ev.type) 
		{
		case SDL_KEYUP:
		{
		    int key = ev.key.keysym.sym;
		    
		    if(key == SDLK_ESCAPE)
			stop = 1;
                    /* PATCH DEMO */
		    else if(key == SDLK_SPACE)
		    {
			dbg_printf("seed changed\n");
			srand(1210669189);
		    }
                    /* PATCH DEMO */
		    else if((key >= SDLK_a && key <= SDLK_z) ||
			    key == SDLK_RETURN ||
			    key == SDLK_BACKSPACE)
		    {
			boggle_highlight(b, key);
			changed = 1;
			processed = true;
		    }
		}
		break;
		}
	    }

	    if(!processed)
	    {
		//changed = 1;
		AG_ProcessEvent(&ev);
	    }

	}
	else if (AG_TAILQ_FIRST(&agTimeoutObjQ) != NULL)
	{
	    /* Advance the timing wheels. */
	    AG_ProcessTimeout(Tr2);
	}
	else if (agView->rCur > agIdleThresh)
	{
	    /* Idle the rest of the time. */
	    SDL_Delay(agView->rCur - agIdleThresh);
	}

    }


    if(thread)
	SDL_WaitThread(thread, NULL);

    SDL_FreeSurface(font);
    SDL_FreeSurface(font2);
    SDL_FreeSurface(boxes);
    SDL_FreeSurface(screen);
    AG_Destroy();
    SDL_Quit();
}

void init()
{
    /* on charge les cases */
    SDL_Surface *temp = SDL_LoadBMP(RES_BOXES);
    c_assert2(temp, "unable to load " RES_BOXES);
    boxes = SDL_ConvertSurface(temp, screen->format, SDL_SWSURFACE);
    c_assert(boxes);
    SDL_FreeSurface(temp);

    temp = SDL_LoadBMP(RES_FONT);
    c_assert2(temp, "unable to load " RES_FONT);
    font = SDL_ConvertSurface(temp, screen->format, SDL_SWSURFACE);
    c_assert(font);
    SDL_FreeSurface(temp);

    temp = SDL_LoadBMP(RES_FONT2);
    c_assert2(temp, "unable to load " RES_FONT2);
    font2 = SDL_ConvertSurface(temp, screen->format, SDL_SWSURFACE);
    c_assert(font2);
    SDL_FreeSurface(temp);

    draw_rect(screen, 0, 0, screen->w, screen->h, BGCOLOR);

    stop = 0;

    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);

}

void render()
{

    if(!changed)
	return;

    changed = 0;

    if(SDL_MUSTLOCK(screen))
	if(SDL_LockSurface(screen) < 0) 
	    return;

    size_t x, y;
    
    /* on trace le plateau */
    for(x = 0; x < boggle_box_xcount(current_board); ++x)
	for(y = 0; y < boggle_box_ycount(current_board); ++y)
	{

	    int xx = x * BOX_SIZE_X + BOARD_START_X;
	    int yy = y * BOX_SIZE_Y + BOARD_START_Y;

	    draw_tile2(screen, boxes,
		       0, boggle_get_flag(current_board, x, y),
		       BOX_SIZE_Y,
		       BOX_SIZE_X,
		       xx, yy);

	    draw_smooth_transparent_tile(screen, font,
					 boggle_get_box(current_board, x, y) - LETTER_FIRST,
					 FONT_SIZE_Y,
					 FONT_SIZE_X,
					 xx + BOX_SIZE_X / 2 - FONT_SIZE_X / 2,
					 yy + BOX_SIZE_Y / 2 - FONT_SIZE_Y / 2, 
					 BGCOLOR, 0.70);
	}

    draw_rect(screen, 10, screen->h - font2->w - 8, screen->w - FOUND_WIN_SIZE_X, font2->w,  BGCOLOR);
    draw_string(screen, font2, 10, screen->h - font2->w - 8, current_board->current);


     if(SDL_MUSTLOCK(screen)) 
	SDL_UnlockSurface(screen);

    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}
