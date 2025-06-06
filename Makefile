
SRCDIR=./src
INCDIR=./include
CC=gcc
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR) 
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net -lSDL2_ttf -lSDL2_mixer -mwindows -mconsole -lm

snake: main.o snake.o bakgrund.o meny.o
	$(CC) -o snake main.o snake.o bakgrund.o meny.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

snake.o: $(SRCDIR)/snake.c $(INCDIR)/snake.h
	$(CC) $(CFLAGS) $(SRCDIR)/snake.c

bakgrund.o: $(SRCDIR)/bakgrund.c $(INCDIR)/bakgrund.h
	$(CC) $(CFLAGS) $(SRCDIR)/bakgrund.c

meny.o: $(SRCDIR)/meny.c $(INCDIR)/meny.h
	$(CC) $(CFLAGS) $(SRCDIR)/meny.c

clean:
	rm *.o
	rm *.exe