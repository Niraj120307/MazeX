// ============================================================================
//  main.cpp  -  MAZE-X console app (no graphics needed)
//    mazex                 demo: all four algorithms on prepared mazes + generator
//    mazex --no-pause      same, without waiting for Enter
//    mazex --bench [file]  benchmark on generated mazes -> CSV (default benchmarks/results.csv)
//  The graphical version is MazeX.exe (src/ui). Tests live in tests/.
// ============================================================================

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Benchmark.h"
#include "ConsoleView.h"
#include "MazeGenerator.h"
#include "Maze.h"
#include "MazeIO.h"
#include "Solvers.h"

namespace {

void runAll(const Maze& maze, bool showGrids) {
    // Every solver receives its own copy of the same maze -> fair comparison.
    std::vector<SolverResult> results = {
        solveDFS(maze), solveBFS(maze), solveDijkstra(maze), solveAStar(maze),
    };
    if (showGrids) {
        for (const SolverResult& r : results) {
            std::cout << "\n";
            printSummary(r);
            printSearch(maze, r);
        }
    }
    std::cout << "\n";
    printComparison(results);
}

void pause(const std::string& msg) {
    std::cout << "\n  \033[90m" << msg << " (press Enter)\033[0m";
    std::string line;
    std::getline(std::cin, line);
}

}  // namespace

int runBenchmarkMode(const std::string& file) {
    std::cout << "\n  Benchmark: sizes 21, 41, 61, 81 x 3 maze types x 4 algorithms, median of 5 runs\n";
    auto rows = runBenchmarks({21, 41, 61, 81}, 5, 2026);
    std::filesystem::path p(file);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    if (!writeBenchmarkCsv(rows, file)) { std::cout << "  Could not write " << file << "\n"; return 1; }
    std::cout << "  size  maze type           algorithm  explored  moves   cost   median ms\n";
    for (const BenchmarkRow& r : rows)
        std::printf("  %4d  %-18s  %-9s  %8d  %5d  %5.0f   %9.4f\n", r.size, r.mazeType.c_str(),
                    r.algorithm.c_str(), r.nodesVisited, r.pathLength, r.pathCost, r.medianMs);
    std::cout << "\n  Wrote " << rows.size() << " rows to " << file << "\n\n";
    return 0;
}

int main(int argc, char* argv[]) {
    setupConsole();
    const std::string arg = argc > 1 ? argv[1] : "";
    if (arg == "--bench") return runBenchmarkMode(argc > 2 ? argv[2] : "benchmarks/results.csv");
    bool interactive = arg != "--no-pause";

    std::cout << "\033[96m\n  MAZE-X  |  V1.0 Core Engine (console)\n"
              << "  DFS · BFS · Dijkstra · A*  on the same maze\033[0m\n";

    // ---- Demo 1: unweighted maze -------------------------------------------
    // Shows: DFS finds A path (not shortest); BFS/Dijkstra/A* find the shortest;
    // A* explores far fewer cells because it is pulled toward the goal.
    Maze maze = mazeFromLines({
        "S.......#...........",
        ".######.#.########..",
        ".#......#........#..",
        ".#.##########.##.#..",
        ".#..........#..#.#..",
        ".##########.##.#.#..",
        "..........#....#....",
        "#########.#.####.###",
        "..........#.........",
        "..................#E",
    });
    std::cout << "\n  Demo 1: unweighted maze (" << maze.getRows() << " x " << maze.getCols() << ")\n";
    maze.print();
    if (interactive) pause("Solve with all four algorithms");
    runAll(maze, true);

    // ---- Demo 2: weighted terrain ------------------------------------------
    // Digits = cost of stepping onto that cell. BFS ignores costs and walks
    // straight through the swamp; Dijkstra and A* go around it.
    if (interactive) pause("Next: weighted terrain");
    Maze terrain = mazeFromLines({
        "..........",
        "..999999..",
        "S.999999.E",
        "..999999..",
        "..999999..",
        "..999999..",
    });
    std::cout << "\n  Demo 2: weighted terrain - straight line crosses cost-9 cells\n";
    terrain.print();
    if (interactive) pause("Solve");
    runAll(terrain, true);

    // ---- Demo 3: no path ----------------------------------------------------
    Maze sealed = mazeFromLines({
        "S.....",
        "....##",
        "....#E",
    });
    std::cout << "\n  Demo 3: End is sealed off\n";
    runAll(sealed, false);

    // ---- Demo 4: generated maze (V0.5) -------------------------------------
    if (interactive) pause("Next: recursive-backtracking generator");
    Maze generated(15, 31);
    generateBacktracker(generated, 2026, 0.10);
    std::cout << "\n  Demo 4: recursive backtracking, seed 2026, 10% extra openings ("
              << (isSolvable(generated) ? "solvable" : "NOT solvable") << ")\n";
    SolverResult gen = solveAStar(generated);
    printSummary(gen);
    printSearch(generated, gen);

    // ---- Save / load round trip ---------------------------------------------
    if (saveMaze(maze, "demo_maze.txt"))
        std::cout << "\n  Demo 1 maze saved to demo_maze.txt (load it with loadMaze).\n\n";
    return 0;
}
