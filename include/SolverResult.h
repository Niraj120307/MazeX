// ============================================================================
//  SolverResult.h  -  The ONE result format every solver returns (V0.3)
//  DFS, BFS, Dijkstra and A* all fill this same struct, so the demo, the tests,
//  the comparison table and (later) the SFML renderer can treat them identically.
// ============================================================================

#pragma once

#include <string>
#include <vector>
#include "Maze.h"

// ---------------------------------------------------------------------------
//  SearchEvent: one step of the search, recorded in the exact order it happened.
//  Replaying these events lets the UI (V0.7/V0.8) animate the search and rebuild
//  the live stack / queue / priority-queue contents WITHOUT re-running the solver.
//    Push   -> a cell was added to the data structure (frontier grows)
//    Expand -> a cell was taken out and explored      (frontier shrinks)
//    Skip   -> a stale duplicate was taken out and ignored (frontier shrinks)
// ---------------------------------------------------------------------------
enum class EventType { Push, Expand, Skip };

struct SearchEvent {
    EventType type;
    Position  cell;
    double    priority;   // cost/f-score for Dijkstra and A*; 0 for DFS/BFS
};

struct SolverResult {
    std::string algorithm;                 // "DFS", "BFS", "Dijkstra", "A*"
    std::string dataStructure;             // "Stack", "Queue", "Min-priority queue"
    bool found = false;                    // was a path found?
    std::vector<Position> path;            // start -> end, in order (empty if not found)
    std::vector<Position> visitedOrder;    // cells in the order they were expanded
    std::vector<SearchEvent> events;       // full push/expand log for animation
    int    nodesVisited = 0;               // number of cells expanded
    int    pathLength   = 0;               // number of MOVES = path cells - 1
    double pathCost     = 0;               // sum of weights entered (== moves on an unweighted maze)
    double elapsedMs    = 0;               // solver time only (no printing/rendering)
    int    maxFrontier  = 0;               // largest size the stack/queue/PQ reached (memory use)
};

// ---------------------------------------------------------------------------
//  Shared helpers (implemented in SolverResult.cpp) - written once, used by all.
// ---------------------------------------------------------------------------

// Walks parent[] from the end back to the start, then reverses.
// parent[i] holds the flat index of the cell that discovered cell i (-1 = none).
std::vector<Position> reconstructPath(const Maze& maze, const std::vector<int>& parent);

// Sum of getWeight() of every cell entered along the path (start excluded).
double computePathCost(const Maze& maze, const std::vector<Position>& path);

// True if path starts at S, ends at E, never touches a wall, and every step is
// one legal Up/Right/Down/Left move. Used by tests to verify EVERY solver.
bool isValidPath(const Maze& maze, const std::vector<Position>& path);

// Fills found / path / pathLength / pathCost once the search loop has finished.
void finishResult(const Maze& maze, const std::vector<int>& parent, bool reachedEnd,
                  SolverResult& result);
