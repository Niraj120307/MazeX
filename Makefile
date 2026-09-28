# Build:  make      Run:  make run      Clean:  make clean
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRC      = src/main.cpp src/Maze.cpp

mazex: $(SRC) include/Cell.h include/Maze.h
	$(CXX) $(CXXFLAGS) $(SRC) -o mazex

run: mazex
	./mazex

clean:
	rm -f mazex