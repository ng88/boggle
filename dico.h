#ifndef DICO_H
#define DICO_H

#include <stdio.h>
#include <stdlib.h>

#include "bool.h"


#define LETTER_FIRST 'a'
#define LETTER_LAST  'z'
#define LETTER_COUNT (LETTER_LAST - LETTER_FIRST + 1)
#define LARGER_WORD 64

typedef enum
{
    A_NOT_FOUND = 0,
    A_BEGIN_MATCH = 1,
    A_PEFECT_MATCH = 2
} ans_t;

typedef struct
{
    FILE * stream;
    long letter_pos[LETTER_COUNT][LETTER_COUNT][LETTER_COUNT];
    long end;
} dico_t;


dico_t * open_dico(char * dico);

void close_dico(dico_t * d);

ans_t word_exists(dico_t * d, char * word);



#endif

