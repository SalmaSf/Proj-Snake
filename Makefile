
SRCDIR=./src
INCDIR=./include
CC=gcc
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR) 
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -mwindows -lm

snake: main.o snake.o bakgrund.o
	$(CC) -o snake main.o snake.o bakgrund.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

snake.o: $(SRCDIR)/snake.c $(INCDIR)/snake.h
	$(CC) $(CFLAGS) $(SRCDIR)/snake.c

bakgrund.o: $(SRCDIR)/bakgrund.c $(INCDIR)/bakgrund.h
	$(CC) $(CFLAGS) $(SRCDIR)/bakgrund.c

clean:
	rm *.o
	rm *.exe