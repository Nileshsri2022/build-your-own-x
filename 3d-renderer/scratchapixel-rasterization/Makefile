CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

rasterizer: rasterizer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f rasterizer

.PHONY: clean
