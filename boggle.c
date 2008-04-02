
#include <string.h>
#include <time.h>

#include "boggle.h"
#include "assert.h"

const char dx[D_COUNT] = { 0, 0, 1, -1,  1, -1, 1, -1};
const char dy[D_COUNT] = {-1, 1, 0,  0, -1, -1, 1,  1};

/*

        N

     -->
    | a b c
 W  v e F g    E
      i j k

        S

*/


board_t * create_board(dico_t * dico, size_t s)
{
    board_t * b = (board_t *)malloc(sizeof(*b));
    c_assert2(b, "malloc failed");

    b->size = s;
    b->wordlist = create_vector(16);
    b->foundword = create_vector(8);
    b->score = 0;
    b->dico = dico;

    b->cs = (box_t**)malloc(sizeof(box_t*) * s);
    c_assert2(b->cs, "malloc failed");

    b->fl = (box_t**)malloc(sizeof(box_t*) * s);
    c_assert2(b->fl, "malloc failed");


    size_t i;
    for(i = 0; i < s; ++i)
    {
	b->cs[i] = (box_t*)malloc(sizeof(box_t) * s);
	c_assert2(b->cs[i], "malloc failed");

	b->fl[i] = (box_t*)malloc(sizeof(box_t) * s);
	c_assert2(b->fl[i], "malloc failed");

    }


    return b;
}

void fill_board(board_t * b)
{
    static bool init = false;
    if(!init)
    {
	srand(time(0));
	init = true;
    }

    c_assert(b);
    size_t i, j;

    for(i = 0; i < box_xcount(b); ++i)
    {
	for(j = 0; j < box_ycount(b); ++j)
	{
	    set_box(b, i, j, 
		    LETTER_FIRST + (int)((double)(LETTER_LAST - LETTER_FIRST) * (rand() / (double)RAND_MAX))
		    );
	}
    }

}

void print_current(board_t * b)
{
    size_t i;
    for(i = 0; i < b->current_size; ++i)
	putchar(b->current[i]);

    putchar('\n');
}

void search_word(board_t * b, size_t i, size_t j)
{
    size_t d;

    if(b->current_size >= LARGER_WORD)
	return;

    if(b->current_size > 2)
    {
        /* does a word begining with 'current' exists ?  */
	print_current(b);
    }

    for(d = 0; d < D_COUNT; ++d)
    {
	if(is_valid_dir(b, i, j, d) && get_flag(b, i, j) == FL_FREE)
	{
	    b->current[b->current_size] = get_box(b, i, j);
	    b->current_size++;
	    set_flag(b, i, j, FL_BUSY);
	    search_word(b, i + dx[d], j + dy[d]);
	    set_flag(b, i, j, FL_FREE);
	    b->current_size--;
	}
    }
}

void create_wordlist(board_t * b)
{
    c_assert(b);
    size_t i, j, d;

    b->current_size = 1;
    for(i = 0; i < box_xcount(b); ++i)
	for(j = 0; j < box_ycount(b); ++j)
	    set_flag(b, i, j, FL_FREE);


    for(i = 0; i < box_xcount(b); ++i)
    {
	for(j = 0; j < box_ycount(b); ++j)
	{
	    b->current[0] = get_box(b, i, j);

	    for(d = 0; d < D_COUNT; ++d)
	    {
		if(is_valid_dir(b, i, j, d) && get_flag(b, i, j) == FL_FREE)
		{
		    printf("%u %u %u\n", i, j, d);
		    set_flag(b, i, j, FL_BUSY);
		    search_word(b, i + dx[d], j + dy[d]);
		    set_flag(b, i, j, FL_FREE);
		}
	    }
	}
    }
}

void free_board(board_t * b)
{
    if(!b) return;

    free_vector(b->foundword, 0);
    free_vector(b->wordlist, 1);

    size_t i;
    for(i = 0; i < b->size; ++i)
    {
	free(b->cs[i]);
	free(b->fl[i]);
    }

    free(b->cs);
    free(b->fl);

    free(b);

}


void print_board(board_t *b)
{
    c_assert(b);
    size_t i, j;

    for(i = 0; i < box_xcount(b); ++i)
    {
	for(j = 0; j < box_ycount(b); ++j)
	{
	    printf("%c ", get_box(b, i, j));
	}
	putchar('\n');
    }

    printf("\nscore: %d\nfound word:\n", board_score(b));

    for(i = 0; i < vector_size(b->foundword); ++i)
    {
	char * w = (char*)vector_get_element_at(b->foundword, i);
	printf("%s (score=%d)\n", w, score_for_word(w));
    }

    puts("\nwordlist:\n");

    for(i = 0; i < vector_size(b->wordlist); ++i)
    {
	char * w = (char*)vector_get_element_at(b->wordlist, i);
	printf("%s (score=%d)\n", w, score_for_word(w));
    }

}


score_t score_for_word(char * word)
{
    c_assert(word);
    switch(strlen(word))
    {
    case 0:
    case 1:
    case 2: return 0;
    case 3:
    case 4: return 1;
    case 5: return 2;
    case 6: return 3;
    case 7: return 5;
    default: return 11;
    }
}

