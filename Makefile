
SRCDIR=./src
INCDIR=./include
CC=gcc
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR) 
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -mwindows -lm

rocket: main.o rocket.o
	$(CC) -o rocket main.o rocket.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

rocket.o: $(SRCDIR)/rocket.c $(INCDIR)/rocket.h
	$(CC) $(CFLAGS) $(SRCDIR)/rocket.c

clean:
	rm *.o
	rm *.exe