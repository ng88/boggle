#include "sdl_draw.h"
#include "assert.h"


void draw_tile(SDL_Surface *screen, SDL_Surface *tiles, int tile, int tile_size_x, int tile_size_y, int x, int y)
{

    if (SDL_MUSTLOCK(tiles))
	if (SDL_LockSurface(tiles) < 0) 
	    return;

    int i, j;
    for (i = 0; i < tile_size_x; i++)
    {
	int screenofs = x + (y + i) * (screen->pitch / 4);
	int tileofs = (i + tile * tile_size_x) * (tiles->pitch / 4);
	for (j = 0; j < tile_size_y; j++)
	{
	    ((unsigned int*)screen->pixels)[screenofs] = 
		((unsigned int*)tiles->pixels)[tileofs];
	    screenofs++;
	    tileofs++;
	}
    }
    
    if (SDL_MUSTLOCK(tiles)) 
        SDL_UnlockSurface(tiles);
}

void draw_transparent_tile(SDL_Surface *screen, SDL_Surface *tiles, int tile, int tile_size_x, int tile_size_y, int x, int y, unsigned int transparent_color)
{

    if (SDL_MUSTLOCK(tiles))
	if (SDL_LockSurface(tiles) < 0) 
	    return;

    int i, j;
    for (i = 0; i < tile_size_x; i++)
    {
	int screenofs = x + (y + i) * (screen->pitch / 4);
	int tileofs = (i + tile * tile_size_x) * (tiles->pitch / 4);
	for (j = 0; j < tile_size_y; j++)
	{
	    if(((unsigned int*)tiles->pixels)[tileofs] != transparent_color)
		((unsigned int*)screen->pixels)[screenofs] = 
		    ((unsigned int*)tiles->pixels)[tileofs];

	    screenofs++;
	    tileofs++;
	}
    }
    
    if (SDL_MUSTLOCK(tiles)) 
        SDL_UnlockSurface(tiles);
}

void draw_tile2(SDL_Surface *screen, SDL_Surface *tiles, int tilex, int tiley, int tile_size_x, int tile_size_y, int x, int y)
{

    if (SDL_MUSTLOCK(tiles))
	if (SDL_LockSurface(tiles) < 0) 
	    return;

    int i, j;
    for (i = 0; i < tile_size_x; i++)
    {
	int screenofs = x + (y + i) * (screen->pitch / 4);
	int tileofs =  tilex * tile_size_y + (i + tiley * tile_size_x) * (tiles->pitch / 4);
	for (j = 0; j < tile_size_y; j++)
	{
	    ((unsigned int*)screen->pixels)[screenofs] = 
		((unsigned int*)tiles->pixels)[tileofs];
	    screenofs++;
	    tileofs++;
	}
    }
    
    if (SDL_MUSTLOCK(tiles)) 
        SDL_UnlockSurface(tiles);
}


void draw_rect(SDL_Surface *screen, int x, int y, int width, int height, int c)
{

    int i, j;
    for (i = 0; i < height; i++)
    {
	if ((y + i) >= 0 && (y + i) < screen->h)
	{
	    int len = width;
	    int xofs = x;

	    if (xofs < 0)
	    {
		len += xofs;
		xofs = 0;
	    }


	    if (xofs + len >= screen->w)
	    {
		len -= (xofs + len) - screen->w;
	    }
	    int ofs = (i + y) * (screen->pitch / 4) + xofs;

	    for (j = 0; j < len; j++)
		((unsigned int*)screen->pixels)[ofs + j] = c;
	}
    }
}

void draw_string(SDL_Surface *screen, SDL_Surface * font, int x, int y, char * s)
{
    if (SDL_MUSTLOCK(font))
	if (SDL_LockSurface(font) < 0) 
	    return;

    int dep_x = x;
    
    while(*s)
    {
	if(*s == '\n')
	{
	    x = dep_x;
	    y += font->w;
	    s++;
	    
	}
	else
	{
	    if(*s >= 33)
	    {
		int i, j;
		for (i = 0; i < font->w; i++)
		{
		    int screenofs = x + (y + i) * (screen->pitch / 4);
		    int charofs = (i + (*s - 33) * font->w) * (font->pitch / 4);
		    for (j = 0; j < font->w; j++)
		    {
			((unsigned int*)screen->pixels)[screenofs] = 
			    ((unsigned int*)font->pixels)[charofs];
			screenofs++;
			charofs++;
		    }
		}
	    }
	    s++;
	    x += font->w;
	}
    }
    
    if (SDL_MUSTLOCK(font)) 
        SDL_UnlockSurface(font);
}
