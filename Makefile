# MAZE-X build
#   make        build the console demo  ->  mazex
#   make run    build + run the console demo
#   make test   build + run all tests
#   make gui    build the SFML 3 GUI     ->  MazeX   (needs SFML 3 installed)
#   make bench  write benchmarks/results.csv
#   make clean  remove binaries
# On Windows with MinGW use:  mingw32-make  (or the g++ lines in README.md)

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude

CORE  = src/Maze.cpp src/MazeIO.cpp src/SolverResult.cpp \
        src/DFS.cpp src/BFS.cpp src/Dijkstra.cpp src/AStar.cpp src/ConsoleView.cpp \
        src/MazeGenerator.cpp src/Playback.cpp src/Benchmark.cpp
TESTS = tests/test_main.cpp tests/test_maze.cpp tests/test_solvers.cpp tests/test_engine.cpp
UI    = src/ui/main_gui.cpp src/ui/App.cpp src/ui/Controls.cpp src/ui/Renderer.cpp src/ui/Widgets.cpp
HDRS  = $(wildcard include/*.h) tests/TestUtil.h
SFML  = -lsfml-graphics -lsfml-window -lsfml-system

mazex: src/main.cpp $(CORE) $(HDRS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(CORE) -o mazex

test_mazex: $(TESTS) $(CORE) $(HDRS)
	$(CXX) $(CXXFLAGS) -Itests $(TESTS) $(CORE) -o test_mazex

MazeX: $(UI) $(CORE) $(HDRS) $(wildcard src/ui/*.h)
	$(CXX) $(CXXFLAGS) -O2 $(UI) $(CORE) -o MazeX $(SFML)

gui: MazeX

bench: mazex
	./mazex --bench benchmarks/results.csv

run: mazex
	./mazex

test: test_mazex
	./test_mazex -q

clean:
	rm -f mazex mazex.exe test_mazex test_mazex.exe MazeX MazeX.exe mazex_console.exe demo_maze.txt

.PHONY: run test gui bench clean
