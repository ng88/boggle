#include <unistd.h>

#include "assert.h"
#include "boggle.h"

#ifdef G_WINDOWS
# include "SDL.h"
#else
# include "SDL/SDL.h"
#endif

#include <agar/core.h>
#include <agar/gui.h>

#include "sdl_draw.h"

#define RES_BOXES "images/carre.bmp"
#define RES_FONT "images/font1.bmp"
#define RES_FONT2 "images/font2.bmp"
#define RES_SEL "res/selecteur.bmp"

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

static int stop;
static int changed;
static SDL_Surface *screen;
static SDL_Surface *font;
static SDL_Surface *font2;
static SDL_Surface *boxes;
static board_t * current_board;

void render(board_t * b);
void init(board_t * b);

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

	int i;
	for(i = 0; i < last_size; ++i)
	{
	    char * w = (char*)vector_get_element_at(current_board->foundword, i);
	    AG_TableAddRow(tbl, "%s:%d", w, score_for_word(w));
	}
 
	AG_TableEnd(tbl);

	printf("maj %d\n", rand());
    }
}

void boggle_start_ihm(board_t * b)
{

    c_assert(b);
    current_board = b;

    if (AG_InitCore("Boggle", 0) == -1) {
	fprintf(stderr, "%s\n", AG_GetError());
	return;
    }
    if (AG_InitVideo(640, 460, 32, 
		     SDL_SWSURFACE | 
		     SDL_DOUBLEBUF
	    ) == -1)
    {
	fprintf(stderr, "%s\n", AG_GetError());
	return;
    }

    screen = agView->v;

    AG_Window *win;
    win = AG_WindowNew(0);
    AG_WindowSetCaption(win, "Information");
    AG_WindowSetGeometry(win, 
			 640 - 200, 0,
			 200, 300);



    //AG_Table * tbl = AG_TableNew(win, AG_TABLE_EXPAND);
    AG_Table * tbl = AG_TableNewPolled(win, AG_TABLE_EXPAND, update_foundtable, NULL);
    AG_TableAddCol(tbl, "Found word", "<LARGER WORD POSS>", &sort_string);
    AG_TableAddCol(tbl, "Score", "<110>", &sort_int);
    
/*
    AG_TableBegin(tbl);

    int i;
    for(i = 0; i < vector_size(b->wordlist); ++i)
    {
	char * w = (char*)vector_get_element_at(b->wordlist, i);
	AG_TableAddRow(tbl, "%s:%d", w, score_for_word(w));
    }



    AG_TableEnd(tbl);
*/

    //AG_WindowSetPosition(win, AG_WINDOW_UPPER_RIGHT, 0);
    AG_WindowShow(win);
    //AG_EventLoop();
    //

    //return;

/*
    if(SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
	fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	return;
    }



    screen = SDL_SetVideoMode(BOARD_START_X + box_xcount(b) * BOX_SIZE_X + BOARD_RIGHT,
			      BOARD_START_Y + box_ycount(b) * BOX_SIZE_Y + BOARD_BOTTOM,
			      32, SDL_SWSURFACE | SDL_DOUBLEBUF);

    SDL_WM_SetCaption("Boggle", NULL);
*/
    if (screen == NULL)
    {
	fprintf(stderr, "Unable to set %dx%d video: %s\n", screen->w, screen->h, SDL_GetError());
	return;
    }

    AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);

    init(b);
  
    boogle_start_game(b);
    changed = 1;
    

    extern struct ag_objectq agTimeoutObjQ;
    SDL_Event ev;
    Uint32 Tr1 = SDL_GetTicks(), Tr2 = 0;
 
    while(!stop)
    {
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
	    switch (ev.type) 
	    {
	    case SDL_KEYUP:
	      {
		  int key = ev.key.keysym.sym;
		  
		  if(key == SDLK_ESCAPE)
		      stop = 1;
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

	    if(!processed)
	    {
		changed = 1;
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


    SDL_FreeSurface(font);
    SDL_FreeSurface(font2);
    SDL_FreeSurface(boxes);
    SDL_FreeSurface(screen);
    AG_Destroy();
    SDL_Quit();
}

void init(board_t * b)
{
    /* on charge les case */
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

void render(board_t * b)
{

    if(!changed)
	return;

    changed = 0;

    if(SDL_MUSTLOCK(screen))
	if(SDL_LockSurface(screen) < 0) 
	    return;

    size_t x, y;
    
    /* on trace le plateau */
    for(x = 0; x < box_xcount(b); ++x)
	for(y = 0; y < box_ycount(b); ++y)
	{

	    int xx = x * BOX_SIZE_X + BOARD_START_X;
	    int yy = y * BOX_SIZE_Y + BOARD_START_Y;

	    draw_tile2(screen, boxes,
		       0, get_flag(b, x, y),
		       BOX_SIZE_Y,
		       BOX_SIZE_X,
		       xx, yy);

	    draw_smooth_transparent_tile(screen, font,
					 get_box(b, x, y) - LETTER_FIRST,
					 FONT_SIZE_Y,
					 FONT_SIZE_X,
					 xx + BOX_SIZE_X / 2 - FONT_SIZE_X / 2,
					 yy + BOX_SIZE_Y / 2 - FONT_SIZE_Y / 2, 
					 BGCOLOR, 0.70);
	}

    draw_rect(screen, 10, screen->h - font2->w - 8, screen->w, font2->w,  BGCOLOR);
    draw_string(screen, font2, 10, screen->h - font2->w - 8, b->current);


     if(SDL_MUSTLOCK(screen)) 
	SDL_UnlockSurface(screen);

    SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}
