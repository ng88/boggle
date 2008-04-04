#include <string.h>

#include "assert.h"
#include "dico.h"

/*
   le dico ne doit contenir que de lettres minuscules [a-z] /!\ /!\ /!\
   les mots doivent etres tries
   les mots de longueur < 3 ne servent a rien au boggle
*/


dico_t * open_dico(char * dico)
{

    dbg_printf("loading dictionnary %s...\n", dico);

    FILE * f = fopen(dico, "r");

    if(!f)
	return NULL;

    dico_t * r = (dico_t*)calloc(1, sizeof(*r));

    if(!r)
	return NULL;

    r->stream = f;

    dbg_printf("generating index for %s...\n", dico);

    char buff[LARGER_WORD];
    char current1 = LETTER_FIRST;
    char current2 = LETTER_FIRST;

    while(!feof(r->stream))
    {
	fgets(buff, LARGER_WORD, r->stream);

	if(buff[0] != current1 || buff[1] != current2)
	{
	    current1 = buff[0];
	    current2 = buff[1];
	    r->letter_pos[current1 - LETTER_FIRST][current2 - LETTER_FIRST] = ftell(r->stream) - strlen(buff);
	}

    }

    r->end = ftell(r->stream);


    int i , j;
    for(i = 0; i < LETTER_COUNT; ++i)
	for(j = 0; j < LETTER_COUNT; ++j)
	{
	    if(r->letter_pos[i][j] == 0L)
		r->letter_pos[i][j] = -1;

	    //printf("%c%c %ld\n", LETTER_FIRST + i, LETTER_FIRST + j, r->letter_pos[i][j]);
	}

    r->letter_pos[0][0] = 0L;

    dbg_printf("%s loaded.\n", dico);
    return r;

}

void close_dico(dico_t * d)
{
    fclose(d->stream);
    free(d);
}


ans_t word_exists(dico_t * d, char * word)
{
    const char *s1, *s2;
    char buff[LARGER_WORD];

    if(word[0] < LETTER_FIRST || word[0] > LETTER_LAST)
	return false;

    if(word[1] < LETTER_FIRST || word[1] > LETTER_LAST)
	return false;

    int c1 = word[0] - LETTER_FIRST;
    int c2 = word[1] - LETTER_FIRST;

    if(d->letter_pos[c1][c2] == -1)
	return false;

    long end;

    do
    {
	if(c2 < LETTER_COUNT - 1)
	    end = d->letter_pos[c1][++c2];
	else
	{
	    if(c1 < LETTER_COUNT - 1)
		end = d->letter_pos[++c1][0];
	    else
		end = d->end;
	}

    } while(end == -1);


    fseek(d->stream, d->letter_pos[word[0] - LETTER_FIRST][word[1] - LETTER_FIRST], SEEK_SET);

    while(ftell(d->stream) < end)
    {
	fgets(buff, LARGER_WORD, d->stream);

	s1 = buff + 2;
	s2 = word + 2;

	while(*s1 != '\n' && *s2 != '\0'  && *s1 == *s2)
	{
	    s1++;
	    s2++;
	}

	if(*s2 == '\0') /* we get at the 
		       end of the input word*/
	{
	    if(*s1 == '\n') /* we get at the
			    end of the dico word*/
		return A_PEFECT_MATCH;
	    else
		return A_BEGIN_MATCH;
	}
	else
	{
	    /** optimization: we know that
	       word are sorted in the dico. */
	    if(*s1 != '\n' && *s2 < *s1)
		return A_NOT_FOUND;
	}

    }

    return A_NOT_FOUND;
    
}
/*

sal      salu
salut


aaabbcd
aaabbc
aacb                aaz
abc
ac
ad
ae
...
*/



