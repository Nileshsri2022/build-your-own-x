CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

raytracer: main.cc *.h
	$(CXX) $(CXXFLAGS) -o raytracer main.cc

test: tests/test_main
	./tests/test_main

tests/test_main: tests/test_main.cc *.h
	$(CXX) $(CXXFLAGS) -o tests/test_main tests/test_main.cc

clean:
	rm -f raytracer tests/test_main

.PHONY: test clean
