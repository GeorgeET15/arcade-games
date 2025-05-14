CC = gcc
CFLAGS = -Iarcade
LDFLAGS_WIN = -lgdi32 -lwinmm
LDFLAGS_LINUX = -lX11 -lm
TARGET = game
SRC = flappybird.c

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "Building for $(shell uname -s)..."
	@if [ "$(shell uname -s)" = "Linux" ]; then 		$(CC) $(SRC) $(CFLAGS) $(LDFLAGS_LINUX) -o $(TARGET); 	else 		$(CC) $(SRC) $(CFLAGS) $(LDFLAGS_WIN) -o $(TARGET).exe; 	fi

clean:
	@rm -f $(TARGET) $(TARGET).exe

run: $(TARGET)
	@if [ "$(shell uname -s)" = "Linux" ]; then 		./$(TARGET); 	else 		./$(TARGET).exe; 	fi

.PHONY: all clean run