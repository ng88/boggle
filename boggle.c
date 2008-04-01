
#include "boggle.h"
#include "assert.h"



board_t * create_board(size_t s)
{
    board_t * b = (board_t *)malloc(sizeof(*b));
    c_assert2(b, "malloc failed");

    b->size = s;
    b->wordlist = create_board(16);
    b->foundword = create_board(8);
    b->score = 0;

    b->cs = (box_t**)malloc(sizeof(box_t*) * s);
    c_assert2(b->cs, "malloc failed");


    size_t i;
    for(i = 0; i < s; ++i)
    {
	b->cs[i] = (box_t*)malloc(sizeof(box_t) * s);
	c_assert2(b->cs[i], "malloc failed");
    }


    return b;
}


void fill_board(board_t * b);

void create_wordlist(board_t * b);

void free_board(board_t * b)
{
    if(!b) return;

    free_vector(b->wordlist, 0);
    free_vector(b->foundword, 0);

    size_t i;
    for(i = 0; i < b->size; ++i)
	free(b->cs[i]);

    free(b->cs);

    free(b);

}


score_t score_for_word(char * word)
{
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

