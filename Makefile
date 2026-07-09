PROJ_NAME=chip8

# folders
SRC=./src
INCLUDE=./include
BUILD=./build

# files
C_SRC=$(wildcard $(SRC)/*.c)
H_SRC=$(wildcard $(INCLUDE)/*.h)

# obj files
OBJ=$(subst .c,.o,$(subst $(SRC),$(BUILD),$(C_SRC)))

# compiler
CC=gcc
CC_FLAGS=-c -Wall -Wextra -Iinclude

# SDL flags
SDL_CFLAGS=$(shell pkg-config --cflags sdl3)
SDL_LIBS=$(shell pkg-config --libs sdl3)

all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	$(CC) $^ $(SDL_LIBS) -o $@

$(BUILD):
	mkdir $(BUILD) -p

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $< $(CC_FLAGS) $(SDL_CFLAGS) -o $@

clean:
	@ rm -rf $(BUILD)/*.o $(PROJ_NAME) ~*
	@ rmdir $(BUILD)

run: all
	./$(PROJ_NAME)

.PHONY: all clean run
