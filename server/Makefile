# Paths
INCDIR=../include
SRCDIR=../src
RESOURCEDIR=../resources

# Compiler and flags
CC=gcc
INCLUDE=-I$(INCDIR) -IC:/msys64/mingw64/include/SDL2
CFLAGS=-g $(INCLUDE) -c
LDFLAGS=-LC:/msys64/mingw64/lib -lSDL2 -lSDL2_image -lSDL2_net -lSDL2_ttf -lm

# Output target
TARGET=snakeServer.exe

# Object files
OBJS=main.o snake.o bakgrund.o

# Build target
$(TARGET): $(OBJS)
	$(CC) -mconsole -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile object files
main.o: main.c $(INCDIR)/snake.h $(INCDIR)/bakgrund.h $(INCDIR)/snake_data.h
	$(CC) $(CFLAGS) -o main.o main.c

snake.o: $(SRCDIR)/snake.c $(INCDIR)/snake.h
	$(CC) $(CFLAGS) -o snake.o $(SRCDIR)/snake.c

bakgrund.o: $(SRCDIR)/bakgrund.c $(INCDIR)/bakgrund.h
	$(CC) $(CFLAGS) -o bakgrund.o $(SRCDIR)/bakgrund.c

# Clean build files
clean:
	del *.o *.exe
