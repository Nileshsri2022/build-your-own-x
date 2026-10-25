# Scratchapixel Introduction to Ray Tracing — C++17, stdlib only.
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

all: raytracer test_raytracer

raytracer: src/main.cpp src/raytracer.hpp
	$(CXX) $(CXXFLAGS) -o raytracer src/main.cpp

test_raytracer: tests/test_raytracer.cpp src/raytracer.hpp
	$(CXX) $(CXXFLAGS) -o test_raytracer tests/test_raytracer.cpp

test: test_raytracer
	./test_raytracer

render: raytracer
	./raytracer render.ppm 200 200

check: raytracer test_raytracer
	./test_raytracer
	python3 tests/check_ppm.py

clean:
	rm -f raytracer test_raytracer render.ppm

.PHONY: all test render check clean
