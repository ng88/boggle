



agar:
	gcc `agar-config --cflags --libs` -W -Wall -Wno-unused -O3 vector.c dico.c boggle.c main.c sdl_draw.c boggle_sdl.c -o boggle

sdl:
	gcc `sdl-config --cflags --libs` -W -Wall -Wno-unused -O3 vector.c dico.c boggle.c tests.c sdl_draw.c boggle_sdl.c
