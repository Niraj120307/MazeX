// ============================================================================
//  main.cpp  -  Stage 1 demo + self-tests for the MAZE-X Core Engine
// ============================================================================

#include <iostream>
#include "Maze.h"

// Windows-only header. On Linux/Mac this whole block is skipped by the preprocessor.
#ifdef _WIN32
    #include <windows.h>
#endif

// ---------------------------------------------------------------------------
//  setupConsole: makes the Windows terminal show box characters and colors.
//  Without this, Windows uses an old code page and "╔═╗" appears as "ΓòöΓòÉ".
// ---------------------------------------------------------------------------
void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);                        // 1) interpret our text as UTF-8

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);       // 2) get the console output handle
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        // 0x0004 = ENABLE_VIRTUAL_TERMINAL_PROCESSING: turns on ANSI colors (\033[92m ...)
        SetConsoleMode(out, mode | 0x0004);
    }
#endif
}

// A tiny test helper: prints PASS/FAIL and counts failures.
static int failures = 0;
void check(bool condition, const std::string& name) {
    std::cout << (condition ? "  \033[92m[PASS]\033[0m " : "  \033[91m[FAIL]\033[0m ") << name << "\n";
    if (!condition) ++failures;
}

int main() {
    setupConsole();   // must run BEFORE any printing

    // ---- Banner --------------------------------------------------------------
    std::cout << "\033[96m\n  MAZE-X  |  Stage 1: Foundation\n"
              << "  Maze class, grid, walls, start/end\033[0m\n\n";

    // ---- Create a 10 x 20 maze ----------------------------------------------
    Maze maze(10, 20);
    maze.setStart(Position(1, 1));
    maze.setEnd(Position(8, 18));

    // Draw a vertical wall at column 6 (rows 0-6) and one at column 12 (rows 3-9).
    for (int r = 0; r <= 6; ++r) maze.setWall(Position(r, 6));
    for (int r = 3; r <= 9; ++r) maze.setWall(Position(r, 12));

    std::cout << "  Maze with two walls:\n";
    maze.print();

    // ---- Self-tests ----------------------------------------------------------
    std::cout << "\n  Running tests...\n";
    check(maze.getRows() == 10 && maze.getCols() == 20,  "size is 10 x 20");
    check(maze.isInside(Position(0, 0)),                 "(0,0) is inside");
    check(!maze.isInside(Position(10, 0)),               "(10,0) is outside");
    check(maze.isWall(Position(2, 6)),                   "(2,6) is a wall");
    check(!maze.setWall(Position(1, 1)),                 "cannot wall over Start");
    check(!maze.setWall(Position(8, 18)),                "cannot wall over End");
    check(!maze.setWall(Position(-1, 5)),                "cannot wall outside grid");
    check(!maze.setEnd(Position(1, 1)),                  "End cannot equal Start");

    // Neighbors of (2,5): Up(1,5) Right(2,6)=WALL Down(3,5) Left(2,4)  -> 3 neighbors
    check(maze.getNeighbors(Position(2, 5)).size() == 3, "(2,5) has 3 neighbors (right is a wall)");
    // Corner (0,0): only Right and Down are legal -> 2 neighbors
    check(maze.getNeighbors(Position(0, 0)).size() == 2, "corner (0,0) has 2 neighbors");

    maze.toggleWall(Position(2, 6));                     // remove that wall
    check(!maze.isWall(Position(2, 6)),                  "toggle removes a wall");
    check(maze.getNeighbors(Position(2, 5)).size() == 4, "(2,5) now has 4 neighbors");

    maze.clearWalls();
    check(!maze.isWall(Position(5, 12)),                 "clearWalls removes all walls");

    bool caught = false;
    try { Maze bad(1, 1); } catch (const std::invalid_argument&) { caught = true; }
    check(caught,                                        "1x1 maze throws an error");

    // ---- Result --------------------------------------------------------------
    std::cout << (failures == 0 ? "\n  \033[92mAll tests passed. Stage 1 complete.\033[0m\n\n"
                                : "\n  \033[91mSome tests failed.\033[0m\n\n");
    return failures;   // 0 = success, non-zero = failure (standard convention)
}