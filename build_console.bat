@echo off
rem ==========================================================================
rem  MAZE-X  -  build the console app + tests with g++ (no SFML needed)
rem  Run from the project folder:  build_console.bat
rem ==========================================================================
setlocal
set FLAGS=-std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude
set CORE=src\Maze.cpp src\MazeIO.cpp src\SolverResult.cpp src\DFS.cpp src\BFS.cpp src\Dijkstra.cpp src\AStar.cpp src\MazeGenerator.cpp src\Playback.cpp src\Benchmark.cpp src\ConsoleView.cpp

where g++ >nul 2>nul || (echo g++ not found on PATH. Install MinGW-w64 / MSYS2 first. & exit /b 1)

echo Building tests...
g++ %FLAGS% -Itests tests\test_main.cpp tests\test_maze.cpp tests\test_solvers.cpp tests\test_engine.cpp %CORE% -o test_mazex.exe || exit /b 1
echo Building console demo...
g++ %FLAGS% src\main.cpp %CORE% -o mazex_console.exe || exit /b 1

echo.
echo Running tests:
test_mazex.exe -q || exit /b 1
echo.
echo Done.  Run:  mazex_console.exe          (demo)
echo              mazex_console.exe --bench  (writes benchmarks\results.csv)
endlocal
