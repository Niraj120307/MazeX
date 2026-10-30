// ============================================================================
//  ConsoleView.cpp  -  Coloured terminal output for MAZE-X
// ============================================================================

#include "ConsoleView.h"
#include <iomanip>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace {
    const char* RESET  = "\033[0m";
    const char* GREEN  = "\033[92m";   // start
    const char* RED    = "\033[91m";   // end
    const char* GRAY   = "\033[90m";   // walls
    const char* CYAN   = "\033[96m";   // frame
    const char* BLUE   = "\033[34m";   // visited (explored) cells
    const char* YELLOW = "\033[93m";   // final path
}

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);                        // show box characters correctly
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
        SetConsoleMode(out, mode | 0x0004);             // ENABLE_VIRTUAL_TERMINAL_PROCESSING
#endif
}

void printSearch(const Maze& maze, const SolverResult& result) {
    // Build the overlay: 0 = untouched, 1 = visited, 2 = on the final path.
    std::vector<int> overlay(maze.cellCount(), 0);
    for (const Position& p : result.visitedOrder) overlay[maze.toIndex(p)] = 1;
    for (const Position& p : result.path)         overlay[maze.toIndex(p)] = 2;

    std::cout << CYAN << "╔";
    for (int c = 0; c < maze.getCols(); ++c) std::cout << "══";
    std::cout << "╗" << RESET << "\n";

    for (int r = 0; r < maze.getRows(); ++r) {
        std::cout << CYAN << "║" << RESET;
        for (int c = 0; c < maze.getCols(); ++c) {
            Position p(r, c);
            int o = overlay[maze.toIndex(p)];
            int w = maze.getWeight(p);
            if      (p == maze.getStart()) std::cout << GREEN << "S " << RESET;
            else if (p == maze.getEnd())   std::cout << RED   << "E " << RESET;
            else if (maze.isWall(p))       std::cout << GRAY  << "██" << RESET;
            else if (o == 2)               std::cout << YELLOW << (w > 1 ? char('0' + w) : '*') << ' ' << RESET;
            else if (o == 1)               std::cout << BLUE  << (w > 1 ? char('0' + w) : 'o') << ' ' << RESET;
            else if (w > 1)                std::cout << w << ' ';
            else                           std::cout << "· ";
        }
        std::cout << CYAN << "║" << RESET << "\n";
    }

    std::cout << CYAN << "╚";
    for (int c = 0; c < maze.getCols(); ++c) std::cout << "══";
    std::cout << "╝" << RESET << "\n";
    std::cout << "  Legend: " << GREEN << "S" << RESET << " start  " << RED << "E" << RESET << " end  "
              << YELLOW << "*" << RESET << " path  " << BLUE << "o" << RESET << " explored  "
              << GRAY << "██" << RESET << " wall  digits = terrain cost\n";
}

void printSummary(const SolverResult& r) {
    std::cout << "  " << r.algorithm << "  [" << r.dataStructure << "]\n";
    if (!r.found) {
        std::cout << "    " << RED << "No path found" << RESET
                  << "  (explored " << r.nodesVisited << " cells)\n";
        return;
    }
    std::cout << std::fixed << std::setprecision(3)
              << "    nodes explored: " << r.nodesVisited
              << "   path length: " << r.pathLength << " moves"
              << "   path cost: " << std::setprecision(0) << r.pathCost
              << "   max frontier: " << r.maxFrontier
              << "   time: " << std::setprecision(3) << r.elapsedMs << " ms\n";
}

void printComparison(const std::vector<SolverResult>& results) {
    std::cout << CYAN
              << "  +-----------+-------+---------+--------+------+----------+-----------+\n"
              << "  | Algorithm | Found | Explored| Moves  | Cost | Frontier | Time (ms) |\n"
              << "  +-----------+-------+---------+--------+------+----------+-----------+\n"
              << RESET;
    for (const SolverResult& r : results) {
        std::cout << "  | " << std::left  << std::setw(9) << r.algorithm
                  << " | " << std::setw(5) << (r.found ? "yes" : "no")
                  << " | " << std::right << std::setw(7) << r.nodesVisited
                  << " | " << std::setw(6) << (r.found ? std::to_string(r.pathLength) : "-")
                  << " | " << std::setw(4) << (r.found ? std::to_string(static_cast<int>(r.pathCost)) : "-")
                  << " | " << std::setw(8) << r.maxFrontier
                  << " | " << std::setw(9) << std::fixed << std::setprecision(3) << r.elapsedMs
                  << " |\n";
    }
    std::cout << CYAN
              << "  +-----------+-------+---------+--------+------+----------+-----------+\n"
              << RESET
              << "  Time is one observed run on this machine (noisy on small mazes);\n"
              << "  compare 'Explored' and 'Cost' first.\n";
}
