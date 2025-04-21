SRCDIR=./src
INCDIR=./include
CC=gcc

INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR)
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -mwindows -lm

snake: main.o snake.o bakgrund.o snake_client.o
	$(CC) -o snake main.o snake.o bakgrund.o snake_client.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

snake.o: $(SRCDIR)/snake.c $(INCDIR)/snake.h
	$(CC) $(CFLAGS) $(SRCDIR)/snake.c

bakgrund.o: $(SRCDIR)/bakgrund.c $(INCDIR)/bakgrund.h
	$(CC) $(CFLAGS) $(SRCDIR)/bakgrund.c

snake_client.o: $(SRCDIR)/snake_client.c $(INCDIR)/snake_client.h
	$(CC) $(CFLAGS) $(SRCDIR)/snake_client.c

clean:
	rm *.o
	rm *.exe
