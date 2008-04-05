#include <unistd.h>

#include "assert.h"
#include "boggle.h"

#ifdef G_WINDOWS
# include "SDL.h"
#else
# include "SDL/SDL.h"
#endif

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

void render(board_t * b);
void init(board_t * b);

void boggle_start_ihm(board_t * b)
{

    if(SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
	fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	return;
    }



    screen = SDL_SetVideoMode(BOARD_START_X + box_xcount(b) * BOX_SIZE_X + BOARD_RIGHT,
			      BOARD_START_Y + box_ycount(b) * BOX_SIZE_Y + BOARD_BOTTOM,
			      32, SDL_SWSURFACE | SDL_DOUBLEBUF);

    SDL_WM_SetCaption("Boggle", NULL);

    if (screen == NULL)
    {
	fprintf(stderr, "Unable to set %dx%d video: %s\n", screen->w, screen->h, SDL_GetError());
	return;
    }
 
    init(b);

    boogle_start_game(b);

    changed = 1;
    
    while(!stop)
    {

	SDL_Event event;
	while (SDL_PollEvent(&event)) 
	{
	    switch (event.type) 
	    {
	    case SDL_KEYUP:
	    {
		int key = event.key.keysym.sym;

		if(key == SDLK_ESCAPE)
		    stop = 1;
		else if((key >= SDLK_a && key <= SDLK_z) ||
			 key == SDLK_RETURN ||
	  		 key == SDLK_BACKSPACE)
		{
		    boggle_highlight(b, key);
		    changed = 1;
		}
	    }
	    break;
	    case SDL_MOUSEMOTION:
		break;
	    case SDL_MOUSEBUTTONDOWN:
		break;
	    case SDL_QUIT:
		stop = 1;
		break;
	    }
	}

	render(b);
    }

    SDL_FreeSurface(font);
    SDL_FreeSurface(font2);
    SDL_FreeSurface(boxes);
    SDL_FreeSurface(screen);
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
