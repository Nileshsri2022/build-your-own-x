CXX ?= /usr/bin/g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Isrc

all: pbrt tests/test_main

pbrt: src/main.cpp src/*.h
	$(CXX) $(CXXFLAGS) src/main.cpp -o pbrt

tests/test_main: tests/test_main.cpp src/*.h
	$(CXX) $(CXXFLAGS) tests/test_main.cpp -o tests/test_main

test: tests/test_main
	./tests/test_main

clean:
	rm -f pbrt tests/test_main

.PHONY: all test clean
