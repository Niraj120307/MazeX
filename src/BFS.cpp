// ============================================================================
//  BFS.cpp  -  Breadth-First Search with a QUEUE (FIFO)
//
//  The queue processes cells in order of distance from the start: first every
//  cell 1 move away, then every cell 2 moves away, and so on. So the FIRST time
//  BFS reaches the end, it has used the fewest possible moves.
//
//  Rule: mark a cell "discovered" when it is ENQUEUED, not when dequeued.
//  Otherwise the same cell can be added to the queue many times.
// ============================================================================

#include "Solvers.h"
#include <algorithm>   // std::max
#include <chrono>
#include <queue>

SolverResult solveBFS(Maze maze) {
    SolverResult result;
    result.algorithm     = "BFS";
    result.dataStructure = "Queue";

    auto t0 = std::chrono::steady_clock::now();

    const int n = maze.cellCount();
    std::vector<bool> discovered(n, false);
    std::vector<int>  parent(n, -1);

    std::queue<int> q;
    const int startIdx = maze.toIndex(maze.getStart());
    const int endIdx   = maze.toIndex(maze.getEnd());

    q.push(startIdx);
    discovered[startIdx] = true;
    result.events.push_back({EventType::Push, maze.getStart(), 0});
    bool reachedEnd = false;

    while (!q.empty()) {
        result.maxFrontier = std::max(result.maxFrontier, static_cast<int>(q.size()));
        int cur = q.front();   // FIFO: oldest entry first
        q.pop();

        Position p = maze.fromIndex(cur);
        result.visitedOrder.push_back(p);
        result.events.push_back({EventType::Expand, p, 0});

        if (cur == endIdx) { reachedEnd = true; break; }

        for (const Position& nb : maze.getNeighbors(p)) {   // Up, Right, Down, Left
            int ni = maze.toIndex(nb);
            if (!discovered[ni]) {
                discovered[ni] = true;   // mark NOW, at enqueue time
                parent[ni]     = cur;    // remember who found it -> path rebuild
                q.push(ni);
                result.events.push_back({EventType::Push, nb, 0});
            }
        }
    }

    finishResult(maze, parent, reachedEnd, result);
    auto t1 = std::chrono::steady_clock::now();
    result.elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}
