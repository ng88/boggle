
#include "boggle.h"

///////////////: TESTS :///////////////


int main()
{
    dico_t * dico  = open_dico("dico_english.txt");

    board_t * b = create_board(dico, 5);


    fill_board(b);


    print_board(b);

    create_wordlist(b);

    print_board(b);

    boggle_start_ihm(b);

    free_board(b);
    close_dico(dico);

    return EXIT_SUCCESS;
}

int main2(int argc, char ** argv)
{


    dico_t * d  = open_dico("dico_english.txt");
    //dico_t * d  = open_dico("d.txt");


    int i,j;

    for(j = 0; j < 100; j++)
	for(i = 1; i < argc; i++)
	{
	    bool b = word_exists(d, argv[i]);
	    printf("%s existe ? %d\n", argv[i], b);
	}


    close_dico(d);

    return EXIT_SUCCESS;
}




/*

100 passes

sans index
==========

time ./a.out hello salut test english anglais voila zero salu hellov > /dev/null

real    0m18.370s
user    0m17.689s
sys     0m0.484s



avec index
==========
time ./a.out hello salut test english anglais voila zero salu hellov > /dev/null
debug> line 24: in dico.c::open_dico(): loading dictionnary dico_en_final.txt...
debug> line 38: in dico.c::open_dico(): generating index for dico_en_final.txt...
debug> line 68: in dico.c::open_dico(): dico_en_final.txt loaded.

real    0m1.213s
user    0m1.192s
sys     0m0.016s


avec index à 2 niveaux
======================
time ./a.out hello salut test english anglais voila zero salu hellov>/dev/null
debug> line 16: in dico.c::open_dico(): loading dictionnary dico_en_final.txt...
debug> line 30: in dico.c::open_dico(): generating index for dico_en_final.txt...
debug> line 64: in dico.c::open_dico(): dico_en_final.txt loaded.

real    0m0.134s
user    0m0.124s
sys     0m0.008s


*/

