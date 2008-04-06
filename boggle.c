
#include <string.h>
#include <time.h>

#include "boggle.h"
#include "assert.h"

const char dx[D_COUNT] = { 0, 0, 1, -1,  1, -1, 1, -1};
const char dy[D_COUNT] = {-1, 1, 0,  0, -1, -1, 1,  1};

#define VOWEL_COUNT 6
const char vowel[VOWEL_COUNT] = {'a', 'e', 'i', 'o', 'u', 'y'};

/*

        N

     -->
    | a b c
 W  v e F g    E
      i j k

        S

*/

void boggle_alloc_boards(board_t * b, size_t s)
{
    b->size = s;

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
}

void boggle_free_boards(board_t * b)
{
    size_t i;
    for(i = 0; i < b->size; ++i)
    {
	free(b->cs[i]);
	free(b->fl[i]);
    }
    
    free(b->cs);
    free(b->fl);
}

board_t * boggle_create_board(dico_t * dico, size_t s)
{
    board_t * b = (board_t *)malloc(sizeof(*b));
    c_assert2(b, "malloc failed");

    
    b->wordlist = create_vector(16);
    b->foundword = create_vector(8);
    b->score = 0;
    b->dico = dico;

    boggle_alloc_boards(b, s);

    return b;
}

void boggle_resize_board(board_t * b, size_t s)
{
    if(s == b->size)
	return;
    else if(s < b->size)
	b->size = s;
    else
    {
	boggle_free_boards(b);
	boggle_alloc_boards(b, s);
    }
}


void boggle_fill_board(board_t * b)
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

    for(i = 0; i < boggle_box_xcount(b); ++i)
    {
	for(j = 0; j < boggle_box_ycount(b); ++j)
	{
	    char letter;

	    if(rand() <= RAND_MAX / 3)
		letter = vowel[(int)((double)(VOWEL_COUNT) * (rand() / (double)RAND_MAX))];		
	    else
		letter = LETTER_FIRST + (int)((double)(LETTER_LAST - LETTER_FIRST) * (rand() / (double)RAND_MAX));

	    boggle_set_box(b, i, j, letter);
	}
    }

}

void add_unique_word_to_list(vector_t * list, char * word)
{
    int s = vector_size(list) - 1;

    if(s > -1)
    {
	int i;
	for(i = s; i > -1; i--)
	{
	    if(!strcmp(word, (char*)vector_get_element_at(list, i)))
		return;
	}
    }

    vector_add_element(list, strdup(word));

}

void boggle_print_current(board_t * b)
{
    size_t i;
    for(i = 0; i < b->current_size; ++i)
	putchar(b->current[i]);

    putchar('\n');
}

void boggle_search_word(board_t * b, size_t i, size_t j)
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
	    add_unique_word_to_list(b->wordlist, b->current);
	    break;
	case A_BEGIN_MATCH:
            /* let's continue */
	    break;
	}
    }

    for(d = 0; d < D_COUNT; ++d)
    {
	if(boggle_is_valid_dir(b, i, j, d) &&
	   boggle_get_flag(b, i, j) == FL_FREE)
	{
	    b->current[b->current_size] = boggle_get_box(b, i, j);
	    b->current_size++;
	    boggle_set_flag(b, i, j, FL_BUSY);
	    boggle_search_word(b, i + dx[d], j + dy[d]);
	    boggle_set_flag(b, i, j, FL_FREE);
	    b->current_size--;
	}
    }
}

int sort_by_score(void * a, void * b)
{
    return boggle_score_for_word((char*)b) -
	boggle_score_for_word((char*)a);
}

void boggle_create_wordlist(board_t * b)
{
    c_assert(b);
    size_t i, j, d;

    clear_vector(b->wordlist, 1);

    b->current_size = 1;
    boggle_reset_flags(b);

    for(i = 0; i < boggle_box_xcount(b); ++i)
    {
	for(j = 0; j < boggle_box_ycount(b); ++j)
	{
	    b->current[0] = boggle_get_box(b, i, j);

	    for(d = 0; d < D_COUNT; ++d)
	    {
		if(boggle_is_valid_dir(b, i, j, d) && 
		   boggle_get_flag(b, i, j) == FL_FREE)
		{
		    boggle_set_flag(b, i, j, FL_BUSY);
		    boggle_search_word(b, i + dx[d], j + dy[d]);
		    boggle_set_flag(b, i, j, FL_FREE);
		}
	    }
	}
    }

    vector_sort(b->wordlist, &sort_by_score);

}

void boggle_start_game(board_t * b)
{
    c_assert(b);

    b->current_size = 0;
    b->current[0] = '\0';
    boggle_reset_flags(b);
    b->score = 0;
    clear_vector(b->foundword, 1);
}

void boggle_reset_flags(board_t * b)
{
    size_t i, j;

    for(i = 0; i < boggle_box_xcount(b); ++i)
	for(j = 0; j < boggle_box_ycount(b); ++j)
	    boggle_set_flag(b, i, j, FL_FREE);
}


void boggle_free_board(board_t * b)
{
    if(!b) return;

    free_vector(b->foundword, 1);
    free_vector(b->wordlist, 1);

    boggle_free_boards(b);

    free(b);

}


void boggle_print_board(board_t *b)
{
    c_assert(b);
    size_t i, j;

    
    for(j = 0; j < boggle_box_ycount(b); ++j)
    {
	for(i = 0; i < boggle_box_xcount(b); ++i)
	{
	    printf("%c ", boggle_get_box(b, i, j));
	}
	putchar('\n');
    }

    printf("\nscore: %d\nfound word:\n", boggle_board_score(b));

    for(i = 0; i < vector_size(b->foundword); ++i)
    {
	char * w = (char*)vector_get_element_at(b->foundword, i);
	printf("%s (score=%d)\n", w, boggle_score_for_word(w));
    }

    puts("\nwordlist:\n");

    for(i = 0; i < vector_size(b->wordlist); ++i)
    {
	char * w = (char*)vector_get_element_at(b->wordlist, i);
	printf("%s (score=%d)\n", w, boggle_score_for_word(w));
    }

}


score_t boggle_score_for_word(char * word)
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

ans_t boggle_word_is_valid(board_t * b, char * word)
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

bool boggle_highlight_path_next(board_t * b, size_t i, size_t j, size_t pos)
{
   pos++;
   if(pos < LARGER_WORD && b->current[pos] == '\0')
   {
       boggle_set_flag(b, i, j, FL_HIGHLIGTHED_END);
       return true;
   }

   size_t d;

   for(d = 0; d < D_COUNT; ++d)
   {
       size_t ii = i + dx[d];
       size_t jj = j + dy[d];

       if(boggle_is_valid_pos(b, ii, jj) &&
	  boggle_get_flag(b, ii, jj) == FL_FREE &&
	  boggle_get_box(b, ii, jj) == b->current[pos])
       {
	   boggle_set_flag(b, ii, jj, FL_HIGHLIGTHED);
	   if(boggle_highlight_path_next(b, ii, jj, pos))
	       return true;
	   boggle_set_flag(b, ii, jj, FL_FREE);
       }
   }

   return false;
}

bool boggle_highlight_path(board_t * b)
{
    c_assert(b);

    size_t i, j;

    boggle_reset_flags(b);

    if(b->current_size == 0)
	return true;

    for(i = 0; i < boggle_box_xcount(b); ++i)
    {
	for(j = 0; j < boggle_box_ycount(b); ++j)
	{
	    if( boggle_get_box(b, i, j) == b->current[0] )
	    {
		boggle_set_flag(b, i, j, FL_HIGHLIGTHED);
		if(boggle_highlight_path_next(b, i, j, 0))
		    return true;
		boggle_set_flag(b, i, j, FL_FREE);
	    }
	}
    }

    return false;
}

void boggle_highlight(board_t * b, char letter)
{
    c_assert(b &&
	     ( letter == KEY_BCKSPACE || letter == KEY_RETURN
	     || (letter >= LETTER_FIRST && letter <= LETTER_LAST)));

    if(letter == KEY_BCKSPACE)
    {
	if(b->current_size > 0)
	{
	    b->current[--b->current_size] = '\0';
	    boggle_highlight_path(b);
	}
    }
    else if(letter == KEY_RETURN)
    {
	if(boggle_word_is_valid(b, b->current)== A_PEFECT_MATCH)
	{
	    size_t old = vector_size(b->foundword);

	    add_unique_word_to_list(b->foundword, b->current);

	    if(old < vector_size(b->foundword))
		b->score += boggle_score_for_word(b->current);

	    b->current[0] = '\0';
	    b->current_size = 0;
	    boggle_reset_flags(b);
	}
    }
    else
    {
	if(b->current_size < LARGER_WORD - 2)
	{
	    b->current[b->current_size++] = letter;
	    b->current[b->current_size] = '\0';

	    if(!boggle_highlight_path(b))
		b->current[--b->current_size] = '\0';

	    boggle_highlight_path(b);
	}  
    }
}

