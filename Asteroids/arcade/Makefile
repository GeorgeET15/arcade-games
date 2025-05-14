
CC = gcc
CFLAGS = -Wall -I"/opt/Arcade IDE/resources/app/lib"
TARGET = arcade.h

ifeq ($(OS),Windows_NT)
    LIBS = -lgdi32 -lwinmm
else
    LIBS = -lX11 -lm
endif

all:
	$(CC) $(CFLAGS) -o "$(TARGET)" "/home/georgeet15/Desktop/arcade-games/Asteroids/arcade/main.c" "/opt/Arcade IDE/resources/app/lib/arcade.c" $(LIBS)

clean:
	rm -f "$(TARGET)" *.o
