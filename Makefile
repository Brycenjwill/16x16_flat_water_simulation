# Compiler
CC = gcc

# Robust SDL2 flags for Linux/WSL using sdl2-config
# This assumes you have run 'sudo apt install libsdl2-dev' in WSL
CFLAGS = $(shell sdl2-config --cflags)
# 🚨 FIX: We grab the SDL libraries and explicitly append the math library (-lm)
LDFLAGS = $(shell sdl2-config --libs) -lm 

# Your program name
TARGET = sim

all: $(TARGET)

# The target now uses the LDFLAGS which includes both -lSDL2 and -lm
$(TARGET): main.o
	$(CC) -o $(TARGET) main.o $(LDFLAGS)

# The object file uses the full CFLAGS from sdl2-config
main.o: main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm -f *.o $(TARGET)