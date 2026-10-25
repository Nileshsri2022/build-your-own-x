# Lodev raycaster (C++, g++). Headless framebuffer build only:
# no window backend is built or claimed (see src/main.cpp).
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

all: raycaster tests

raycaster: src/main.cpp src/raycaster.h
	$(CXX) $(CXXFLAGS) src/main.cpp -o raycaster

tests: src/tests.cpp src/raycaster.h
	$(CXX) $(CXXFLAGS) src/tests.cpp -o tests

test: tests
	./tests

run: raycaster
	./raycaster --mode textured --output /tmp/lodev-frame.ppm
	./raycaster --mode untextured --output /tmp/lodev-frame-untextured.ppm

clean:
	rm -f raycaster tests
