// ============================================================================
//  AStar.cpp  -  A* search: Dijkstra + a heuristic that points at the goal
//
//      f(n) = g(n) + h(n)
//      g(n) = real cost from the start to n (same as Dijkstra's dist)
//      h(n) = ESTIMATED remaining cost from n to the end
//
//  h(n) = Manhattan distance x cheapest step cost in the maze.
//  With 4-direction moves you need at least |dr| + |dc| steps, and each step
//  costs at least minWeight, so h never OVERestimates -> it is "admissible" and
//  A* returns the same optimal cost as Dijkstra while usually exploring fewer cells.
//  The heuristic is also "consistent", so a settled cell never needs re-opening.
//
//  Tie-break: among equal f, prefer smaller h (the cell closer to the goal),
//  then insertion order. This keeps runs repeatable.
// ============================================================================

#include "Solvers.h"
#include <algorithm>   // std::max
#include <chrono>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>

int manhattan(const Position& a, const Position& b) {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

SolverResult solveAStar(Maze maze) {
    SolverResult result;
    result.algorithm     = "A*";
    result.dataStructure = "Min-priority queue (by f = g + h)";

    auto t0 = std::chrono::steady_clock::now();

    const int n   = maze.cellCount();
    const int INF = std::numeric_limits<int>::max();
    const Position goal = maze.getEnd();
    const int stepMin   = maze.minWeight();
    auto h = [&](const Position& p) { return manhattan(p, goal) * stepMin; };

    std::vector<int>  g(n, INF);
    std::vector<int>  parent(n, -1);
    std::vector<bool> settled(n, false);

    // Entry = (f, h, insertion counter, cell)
    using Entry = std::tuple<int, int, int, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
    int counter = 0;

    const int startIdx = maze.toIndex(maze.getStart());
    const int endIdx   = maze.toIndex(goal);

    g[startIdx] = 0;
    int h0 = h(maze.getStart());
    pq.push({h0, h0, counter++, startIdx});
    result.events.push_back({EventType::Push, maze.getStart(), static_cast<double>(h0)});
    bool reachedEnd = false;

    while (!pq.empty()) {
        result.maxFrontier = std::max(result.maxFrontier, static_cast<int>(pq.size()));
        auto [f, hv, order, cur] = pq.top();
        pq.pop();
        (void)hv; (void)order;

        if (settled[cur]) {           // stale entry
            result.events.push_back({EventType::Skip, maze.fromIndex(cur), static_cast<double>(f)});
            continue;
        }
        settled[cur] = true;

        Position p = maze.fromIndex(cur);
        result.visitedOrder.push_back(p);
        result.events.push_back({EventType::Expand, p, static_cast<double>(f)});

        if (cur == endIdx) { reachedEnd = true; break; }

        for (const Position& nb : maze.getNeighbors(p)) {
            int ni = maze.toIndex(nb);
            if (settled[ni]) continue;
            int newG = g[cur] + maze.getWeight(nb);
            if (newG < g[ni]) {
                g[ni]      = newG;
                parent[ni] = cur;
                int hn = h(nb);
                pq.push({newG + hn, hn, counter++, ni});
                result.events.push_back({EventType::Push, nb, static_cast<double>(newG + hn)});
            }
        }
    }

    finishResult(maze, parent, reachedEnd, result);
    auto t1 = std::chrono::steady_clock::now();
    result.elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}
