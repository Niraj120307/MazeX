// ============================================================================
//  Benchmark.cpp  -  Median timing, all-algorithm comparison, CSV export
// ============================================================================

#include "Benchmark.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include "MazeGenerator.h"

SolverResult runTimed(SolverFn solver, const Maze& maze, int repeats) {
    repeats = std::max(1, repeats);
    SolverResult first = solver(maze);
    std::vector<double> times = {first.elapsedMs};
    for (int i = 1; i < repeats; ++i) times.push_back(solver(maze).elapsedMs);
    std::sort(times.begin(), times.end());
    std::size_t n = times.size();
    first.elapsedMs = (n % 2 == 1) ? times[n / 2] : (times[n / 2 - 1] + times[n / 2]) / 2.0;
    return first;
}

std::vector<SolverResult> compareAll(const Maze& maze, int repeats) {
    std::vector<SolverResult> results;
    for (const AlgorithmInfo& a : allAlgorithms())
        results.push_back(runTimed(a.solve, maze, repeats));   // same snapshot for everyone
    return results;
}

std::vector<BenchmarkRow> runBenchmarks(const std::vector<int>& sizes, int repeats, unsigned seed) {
    std::vector<BenchmarkRow> rows;
    for (int size : sizes) {
        for (int type = 0; type < 3; ++type) {
            Maze maze(size, size);
            std::string name;
            if (type == 0) { generateBacktracker(maze, seed + size, 0.10); name = "backtracker+loops"; }
            else if (type == 1) {
                maze.setStartEnd(Position(0, 0), Position(size - 1, size - 1));
                generateRandomObstacles(maze, seed + size, 0.28);
                name = "obstacles";
            } else {
                generateBacktracker(maze, seed + size, 0.25);
                addRandomTerrain(maze, seed + size, 0.30);
                name = "terrain";
            }
            for (const SolverResult& r : compareAll(maze, repeats))
                rows.push_back({size, name, r.algorithm, r.found, r.nodesVisited, r.pathLength,
                                r.pathCost, r.maxFrontier, r.elapsedMs, repeats});
        }
    }
    return rows;
}

bool writeBenchmarkCsv(const std::vector<BenchmarkRow>& rows, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) return false;
    out << "size,maze_type,algorithm,found,nodes_explored,path_moves,path_cost,max_frontier,median_ms,repeats\n";
    out << std::fixed << std::setprecision(4);
    for (const BenchmarkRow& r : rows)
        out << r.size << ',' << r.mazeType << ',' << r.algorithm << ',' << (r.found ? "yes" : "no") << ','
            << r.nodesVisited << ',' << r.pathLength << ',' << r.pathCost << ',' << r.maxFrontier << ','
            << r.medianMs << ',' << r.repeats << '\n';
    return static_cast<bool>(out);
}
