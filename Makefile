PROJ_NAME=chip8

# folders
SRC=./src
INCLUDE=./include
BUILD=./build
BUILD_OBJ=$(BUILD)/obj

# files
C_SRC=$(shell find $(SRC) -name "*.c")
OBJ=$(patsubst $(SRC)/%.c,$(BUILD_OBJ)/%.o,$(C_SRC))
DEP=$(OBJ:.o=.d)
TARGET=$(BUILD)/chip8

# compiler
CC=gcc
CC_FLAGS=-c -Wall -Wextra -Wswitch-enum -I$(INCLUDE) -MMD -MP
LD_FLAGS=-lm

# debug/release
BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE),debug)
	CC_FLAGS += -g -O0
else
	CC_FLAGS += -O2
endif

# SDL flags
SDL_CFLAGS=$(shell pkg-config --cflags sdl3)
SDL_LIBS=$(shell pkg-config --libs sdl3)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(SDL_LIBS) $(LD_FLAGS) -o $@

$(BUILD_OBJ)/%.o: $(SRC)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) $(SDL_CFLAGS) -o $@

-include $(DEP)

clean:
	@ rm -rf $(BUILD)

.PHONY: all clean
