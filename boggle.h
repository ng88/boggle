#ifndef BOGGLE_H
#define BOGGLE_H

#include <stdlib.h>
#include "vector.h"
#include "dico.h"

typedef char box_t;

typedef unsigned int score_t;

typedef enum
{
    D_NORTH = 0,
    D_SOUTH = 1,
    D_EAST = 2,
    D_WEST = 3,
    D_NE = 4,
    D_NW = 5,
    D_SE = 6,
    D_SW = 7,
    D_COUNT = 8
} dir_t;

enum 
{
    FL_FREE = 0,   /* free box */
    FL_HIGHLIGTHED = 1,   /* box which is a part of a word */
    FL_HIGHLIGTHED_END = 2,   /* box which is the end of a word */
    FL_BUSY = 3,   /* busy box */
};

enum
{
    KEY_RETURN = 13,
    KEY_BCKSPACE = 8,
};

const char dx[D_COUNT];
const char dy[D_COUNT];

typedef struct
{
    size_t size;
    box_t ** cs;
    vector_t * wordlist;
    vector_t * foundword;
    score_t score;
    dico_t * dico;

    char current[LARGER_WORD];
    size_t current_size;
    box_t ** fl;
} board_t;


#define boggle_get_box(b, i, j)       ((b)->cs[i][j])
#define boggle_set_box(b, i, j, v)    ((b)->cs[i][j] = (v))
#define boggle_get_flag(b, i, j)      ((b)->fl[i][j])
#define boggle_set_flag(b, i, j, v)   ((b)->fl[i][j] = (v))
#define boggle_box_xcount(b)          ((b)->size)
#define boggle_box_ycount(b)          boggle_box_xcount(b)
#define boggle_box_count(b)           (boggle_box_xcount(b) * boggle_box_ycount(b))
#define boggle_board_score(b)         ((b)->score)

/** return non zero if (i, j) is a valid position */
#define boggle_is_valid_pos(b, i, j) \
       ((i) < boggle_box_xcount(b) && (j) < boggle_box_ycount(b))

/** return non zero if d is a valid direction at (i, j) */
#define boggle_is_valid_dir(b, i, j, d) \
        boggle_is_valid_pos((b), (i) + dx[d], (j) + dy[d])

board_t * boggle_create_board(dico_t * dico, size_t s);

void boggle_fill_board(board_t * b);

void boggle_create_wordlist(board_t * b);

void boggle_start_game(board_t * b);

void boggle_free_board(board_t * b);

void boggle_print_current(board_t * b);
void boggle_print_board(board_t *b);

score_t boggle_score_for_word(char * word);

void boggle_start_ihm(board_t * b);

void boggle_reset_flags(board_t * b);

bool boggle_word_is_valid(board_t * b, char * word);

/** highlight letter 'letter' if it matches a word in the grid.
    letter can be [a-z], backspace or return.
 */
void boggle_highlight(board_t * b, char letter);

bool boggle_highlight_path(board_t * b);

void boggle_highlight_fullword(board_t * b, char * w);


void boggle_resize_board(board_t * b, size_t s);

void boggle_search_word(board_t * b, size_t i, size_t j);

void add_unique_word_to_list(vector_t * list, char * word);

#endif
