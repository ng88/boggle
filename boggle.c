
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
	//srand(23468798);
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

void add_current_to_wordlist(board_t * b)
{
    int s = vector_size(b->wordlist) - 1;

    if(s > -1)
    {
	int i;
	for(i = s; i > -1; i--)
	{
	    char * w = (char*)vector_get_element_at(b->wordlist, i);
	    if(!strcmp(b->current, w))
		return;
	}
    }

    vector_add_element(b->wordlist, strdup(b->current));

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

	b->current[b->current_size] = '\0';
	switch(word_exists(b->dico, b->current))
	{
	case A_NOT_FOUND:
	    /* cancel ...*/
	    return;
	case A_PEFECT_MATCH:
	    add_current_to_wordlist(b);
	    break;
	case A_BEGIN_MATCH:
            /* let's continue */
	    break;
	}
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

    clear_vector(b->wordlist);

    b->current_size = 1;
    boggle_reset_flags(b);

    for(i = 0; i < box_xcount(b); ++i)
    {
	for(j = 0; j < box_ycount(b); ++j)
	{
	    b->current[0] = get_box(b, i, j);

	    for(d = 0; d < D_COUNT; ++d)
	    {
		if(is_valid_dir(b, i, j, d) && get_flag(b, i, j) == FL_FREE)
		{
		    set_flag(b, i, j, FL_BUSY);
		    search_word(b, i + dx[d], j + dy[d]);
		    set_flag(b, i, j, FL_FREE);
		}
	    }
	}
    }
}

void start_game(board_t * b)
{
    c_assert(b);

    b->current_size = 0;
    boggle_reset_flags(b);
    b->score = 0;
    clear_vector(b->foundword);
}

void boggle_reset_flags(board_t * b)
{
    size_t i, j;

    for(i = 0; i < box_xcount(b); ++i)
	for(j = 0; j < box_ycount(b); ++j)
	    set_flag(b, i, j, FL_FREE);
}


void free_board(board_t * b)
{
    if(!b) return;

    free_vector(b->foundword, 1);
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

    
    for(j = 0; j < box_ycount(b); ++j)
    {
	for(i = 0; i < box_xcount(b); ++i)
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

ans_t boogle_word_is_valid(board_t * b, char * word)
{
    c_assert(b && word);

    size_t i;

    for(i = 0; i < vector_size(b->wordlist); ++i)
    {
	char * s1 = (char*)vector_get_element_at(b->wordlist, i);
	char * s2 = word;

	while(*s1 != '\0' && *s2 != '\0'  && *s1 == *s2)
	{
	    s1++;
	    s2++;
	}

	if(*s2 == '\0') /* we get at the 
		       end of the input word*/
	{
	    if(*s1 == '\0') /* we get at the
			    end of the dico word*/
		return A_PEFECT_MATCH;
	    else
		return A_BEGIN_MATCH;
	}
    }

    return A_NOT_FOUND;
}

void boggle_highlight(board_t * b, char letter)
{
    c_assert(b && letter == KEY_BCKSPACE && letter == KEY_RETURN
	     && letter >= LETTER_FIRST && letter <= LETTER_LAST);

    if(letter == KEY_BCKSPACE)
    {
    }
    else if(letter == KEY_RETURN)
    {
	if(boogle_word_is_valid(b, b->current)== A_PEFECT_MATCH)
	{
	    vector_add_element(b->foundword, srtrdup(b->current));
	    b->score += score_for_word(b->current);
	    b->current[0] = '\0';
	    b->current_size = 0;
	    boggle_reset_flags(b);
	}
    }
    else
    {
	if(b->current_size < LARGER_WORD - 2)
	{
	    b->current[b->current_size++] == letter;
	    b->current[b->current_size] == '\0';


	    switch(boogle_word_is_valid(b, b->current))
	    {
	    case A_NOT_FOUND:
		b->current[--b->current_size] == '\0';
		return;
	    case A_PEFECT_MATCH:

	    break;
	    case A_BEGIN_MATCH:
		/* let's continue */
		break;
	    }

	}  
    }
}

