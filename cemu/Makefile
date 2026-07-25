PROJ_NAME=cemu
BUILD_TYPE ?= debug

# folders
SRC=./src
INCLUDE=./include
BIN=../bin
BUILD=../build/$(BUILD_TYPE)/$(PROJ_NAME)
BUILD_OBJ=$(BUILD)/obj

# files
C_SRC=$(shell find $(SRC) -name "*.c")
OBJ=$(patsubst $(SRC)/%.c,$(BUILD_OBJ)/%.o,$(C_SRC))
DEP=$(OBJ:.o=.d)
TARGET=$(BIN)/$(PROJ_NAME)

# compiler
CC=gcc
CC_FLAGS=-c -Wall -Wextra -I$(INCLUDE) -MMD -MP
LD_FLAGS=-lm

# debug/release flags
ifeq ($(BUILD_TYPE),debug)
	CC_FLAGS += -g -O0
else ifeq ($(BUILD_TYPE),release)
	CC_FLAGS += -O2
else
    $(error BUILD_TYPE must be 'release' or 'debug')
endif

# SDL flags
SDL_CFLAGS=$(shell pkg-config --cflags sdl3)
SDL_LIBS=$(shell pkg-config --libs sdl3)

all: $(TARGET)

$(TARGET): $(OBJ)
	@ mkdir -p $(dir $@)
	$(CC) $^ $(SDL_LIBS) $(LD_FLAGS) -o $@

$(BUILD_OBJ)/%.o: $(SRC)/%.c
	@ mkdir -p $(dir $@)
	$(CC) $< $(CC_FLAGS) $(SDL_CFLAGS) -o $@

-include $(DEP)

.PHONY: all
