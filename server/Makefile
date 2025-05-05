INCDIR=../include
SRCDIR=.

CC=gcc
INCLUDE = -I/opt/homebrew/include/SDL2 -I$(INCDIR) 
CFLAGS = -g $(INCLUDE) -c -arch arm64
LDFLAGS = -L/opt/homebrew/lib -lSDL2 -lSDL2_net -lm -arch arm64

snakeServer: server.o 
	$(CC) -o snakeServer server.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c $(INCDIR)/server.h
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

server.o: $(SRCDIR)/server.c $(INCDIR)/server.h
	$(CC) $(CFLAGS) $(SRCDIR)/server.c

clean:
	rm -f *.o snakeServer
