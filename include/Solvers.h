// ============================================================================
//  Solvers.h  -  The four pathfinding algorithms of MAZE-X
//
//  Every solver takes the maze BY VALUE (a snapshot copy), so all algorithms
//  get identical input and none of them can change the user's maze.
//
//  The big idea: all four share the same loop -
//      take a cell out of a container, expand its neighbours, put new cells in.
//  ONLY the container changes:
//      DFS      -> Stack              (LIFO: newest cell first  -> goes deep)
//      BFS      -> Queue              (FIFO: oldest cell first  -> rings outward)
//      Dijkstra -> Min-priority queue (cheapest cost-so-far g)
//      A*       -> Min-priority queue (smallest f = g + h, pulled toward the goal)
// ============================================================================

#pragma once

#include "Maze.h"
#include "SolverResult.h"

SolverResult solveDFS(Maze maze);        // finds A path, not necessarily the shortest. O(V + E)
SolverResult solveBFS(Maze maze);        // fewest moves (ignores weights).            O(V + E)
SolverResult solveDijkstra(Maze maze);   // minimum total cost, weights >= 1.          O((V + E) log V)
SolverResult solveAStar(Maze maze);      // minimum total cost, usually fewer nodes.   O((V + E) log V) here

// ---------------------------------------------------------------------------
//  Registry: lets the UI, comparison and benchmark loop over every algorithm.
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

using SolverFn = SolverResult (*)(Maze);

struct AlgorithmInfo {
    std::string name;            // "DFS"
    SolverFn    solve;           // solveDFS
    std::string dataStructure;   // "Stack"
    std::string complexity;      // "O(V + E)"
    std::string guarantee;       // one-line explanation for the UI / viva
};

const std::vector<AlgorithmInfo>& allAlgorithms();

// Manhattan distance |dr| + |dc|: the fewest moves possible with 4-direction movement.
int manhattan(const Position& a, const Position& b);
