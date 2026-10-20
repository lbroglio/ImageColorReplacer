BUILD_DIR ?= build

.PHONY: all configure build clean

# Default target runs when you just type 'make'
all: build

configure:
	cmake -S . -B $(BUILD_DIR) 

build: configure
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)