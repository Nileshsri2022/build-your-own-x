# Thin wrapper over CMake (the build the docs specify).
BUILD_DIR ?= build
BIN = $(BUILD_DIR)/tinyrenderer

all: $(BIN)

$(BIN): src/*.cpp src/*.h CMakeLists.txt
	cmake -S . -B $(BUILD_DIR) > /dev/null
	cmake --build $(BUILD_DIR) -j

test: all
	python3 tests/run_tests.py

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
