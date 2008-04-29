
#include <time.h>

#include "boggle.h"
#include "assert.h"


#define DEFAULT_DIC "english1.txt"

int main(int argc, char ** argv)
{
    dico_t * dico  = open_dico(argc > 1 ? argv[1] : DEFAULT_DIC);

    if(!dico)
    {
	fputs("unable to open the dictionary!\n", stderr);
	return EXIT_FAILURE;
    }

    unsigned int seed = time(0);

    if(argc > 2)
	seed = atoi(argv[2]);

    boggle_init_rnd(seed);

    dbg_printf("game seed = %d\n", seed);


    board_t * b = boggle_create_board(dico, 7);

    boggle_start_ihm(b);

    boggle_free_board(b);
    close_dico(dico);

    return EXIT_SUCCESS;
}






