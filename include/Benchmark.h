// ============================================================================
//  Benchmark.h  -  Fair comparisons and repeated timing (V0.9 / V1.0)
//
//  Fairness rules (from the project plan):
//   - every algorithm runs on an identical copy of the same maze
//   - time = solver only, repeated, and the MEDIAN is reported
//   - nodes explored and path cost are reported next to time
// ============================================================================

#pragma once

#include <string>
#include <vector>
#include "Maze.h"
#include "Solvers.h"

// Runs one solver `repeats` times on copies of the maze. Returns the result of
// the first run with elapsedMs replaced by the MEDIAN of all runs.
SolverResult runTimed(SolverFn solver, const Maze& maze, int repeats);

// All four algorithms on the same maze snapshot (median timing).
std::vector<SolverResult> compareAll(const Maze& maze, int repeats = 5);

struct BenchmarkRow {
    int size;                 // grid is size x size
    std::string mazeType;     // "backtracker+loops", "obstacles", "terrain"
    std::string algorithm;
    bool found;
    int nodesVisited;
    int pathLength;
    double pathCost;
    int maxFrontier;
    double medianMs;
    int repeats;
};

// Generates one maze per (size, type) with a fixed seed and runs every algorithm.
std::vector<BenchmarkRow> runBenchmarks(const std::vector<int>& sizes, int repeats, unsigned seed);

bool writeBenchmarkCsv(const std::vector<BenchmarkRow>& rows, const std::string& filename);
