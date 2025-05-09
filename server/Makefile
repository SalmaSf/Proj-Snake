# Kataloger
SRCDIR=.
INCDIR=../include
SRCOTH=../src
CC=gcc

# Inkluderingsflaggor
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR)
CFLAGS = -g -Dmain=SDL_main $(INCLUDE) -c

# Länkningsflaggor
LDFLAGS = -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net -lSDL2_ttf -lSDL2_mixer -mwindows -mconsole -lm

# Mål
TARGET = main.exe
OBJS = main.o snake.o bakgrund.o

# Regler
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(SRCDIR)/main.c -o $@

snake.o: $(SRCOTH)/snake.c $(INCDIR)/snake.h
	$(CC) $(CFLAGS) $(SRCOTH)/snake.c -o $@

bakgrund.o: $(SRCOTH)/bakgrund.c $(INCDIR)/bakgrund.h
	$(CC) $(CFLAGS) $(SRCOTH)/bakgrund.c -o $@

clean:
	del /Q *.o *.exe