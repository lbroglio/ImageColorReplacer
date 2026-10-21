BUILD_DIR ?= build
COMPILER_FLAGS ?= -DCMAKE_C_COMPILER=gcc-16 -DCMAKE_CXX_COMPILER=g++-16

.PHONY: all configure build clean

# Default target runs when you just type 'make'
all: build

configure:
	cmake -S . -B $(BUILD_DIR)  $(COMPILER_FLAGS)

build: configure
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)