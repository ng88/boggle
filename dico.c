#include <string.h>

#include "assert.h"
#include "dico.h"


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
    char current = LETTER_FIRST;

    r->letter_pos[0] = 0L;

    while(!feof(r->stream))
    {
	fgets(buff, LARGER_WORD, r->stream);

	if(buff[0] != current)
	{
	    long pos = ftell(r->stream) - strlen(buff);
	    char c;
	    for(c = current + 1; c <= buff[0]; ++c)
		r->letter_pos[c - LETTER_FIRST] = pos;

	    current = buff[0];
	}

    }

    r->letter_pos[LETTER_COUNT] = ftell(r->stream);
/*

    int i;
    for(i = 0; i < LETTER_COUNT; ++i)
	printf("%c %ld\n", LETTER_FIRST + i, r->letter_pos[i]);
*/
    dbg_printf("%s loaded.\n", dico);
    return r;

}

void close_dico(dico_t * d)
{
    fclose(d->stream);
    free(d);
}


bool word_exists(dico_t * d, char * word)
{
    const char *s1, *s2;
    char buff[LARGER_WORD];

    if(word[0] < LETTER_FIRST || word[0] > LETTER_LAST)
	return false;

    long end = d->letter_pos[word[0] - LETTER_FIRST + 1];

    fseek(d->stream, d->letter_pos[word[0] - LETTER_FIRST], SEEK_SET);

    while(ftell(d->stream) < end)
    {
	fgets(buff, LARGER_WORD, d->stream);

	s1 = buff;
	s2 = word;

	while(*s1 != '\n' && *s2 != '\0'  && *s1 == *s2)
	{
	    s1++;
	    s2++;
	}

	if(*s1 == '\n' && *s2 == '\0')
	    return true;

    }

    return false;
    
}







