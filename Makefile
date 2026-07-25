# folders
BUILD=./build
BIN=./bin

all: debug

debug:
	$(MAKE) BUILD_TYPE=debug cemu casm

release:
	$(MAKE) BUILD_TYPE=release cemu casm

cemu:
	$(MAKE) -C cemu BUILD_TYPE=$(BUILD_TYPE)

casm:
	$(MAKE) -C casm BUILD_TYPE=$(BUILD_TYPE)

clean:
	rm -rf $(BUILD)
	rm -rf $(BIN)

.PHONY: all debug release cemu casm
