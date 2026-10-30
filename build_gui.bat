@echo off
rem ==========================================================================
rem  MAZE-X  -  build the SFML GUI (MazeX.exe) with g++ from MSYS2 UCRT64
rem
rem  One-time setup (in the "MSYS2 UCRT64" terminal):
rem     pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sfml
rem  and make sure C:\msys64\ucrt64\bin is on your Windows PATH.
rem
rem  Then, from the project folder:  build_gui.bat
rem  (No MSYS2? Use build_cmake.bat instead - it builds SFML for you.)
rem ==========================================================================
setlocal
set FLAGS=-std=c++17 -Wall -Wextra -pedantic -O2 -Iinclude
set CORE=src\Maze.cpp src\MazeIO.cpp src\SolverResult.cpp src\DFS.cpp src\BFS.cpp src\Dijkstra.cpp src\AStar.cpp src\MazeGenerator.cpp src\Playback.cpp src\Benchmark.cpp src\ConsoleView.cpp
set UI=src\ui\main_gui.cpp src\ui\App.cpp src\ui\Controls.cpp src\ui\Renderer.cpp src\ui\Widgets.cpp

where g++ >nul 2>nul || (echo g++ not found on PATH. & exit /b 1)

echo Building MazeX.exe ...
g++ %FLAGS% %UI% %CORE% -o MazeX.exe -lsfml-graphics -lsfml-window -lsfml-system
if errorlevel 1 (
    echo.
    echo Build failed. Most common causes:
    echo   - SFML 3 not installed:  pacman -S mingw-w64-ucrt-x86_64-sfml   ^(in MSYS2 UCRT64^)
    echo   - g++ on PATH is not the MSYS2 UCRT64 one:  where g++  should show C:\msys64\ucrt64\bin\g++.exe
    echo   - or use build_cmake.bat, which downloads and builds SFML with your compiler
    exit /b 1
)
echo.
echo Done. Run:  MazeX.exe
endlocal
