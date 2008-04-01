#ifndef BOGGLE_H
#define BOGGLE_H

#include <stdlib.h>
#include "vector.h"

typedef char box_t;

typedef unsigned int score_t;

typedef struct
{
    size_t size;
    box_t ** cs;
    vector_t * wordlist;
    vector_t * foundword;
    score_t score;
} board_t;
 

board_t * create_board(size_t s);

void fill_board(board_t * b);

void create_wordlist(board_t * b);

void free_board(board_t * b);


score_t score_for_word(char * word);


#endif
