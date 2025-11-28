# Compiler
CC = clang

CFLAGS = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2

# Output program name
TARGET = sim

all: $(TARGET)

$(TARGET): main.o
	$(CC) main.o $(LDFLAGS) -o $(TARGET)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm -f *.o $(TARGET)
