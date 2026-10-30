// ============================================================================
//  Dijkstra.cpp  -  Dijkstra's algorithm with a MIN-PRIORITY QUEUE
//
//  dist[v] = cheapest known cost from the start to v.
//  The priority queue always hands back the cell with the SMALLEST dist, and
//  because every weight is >= 1 (never negative), that cell's dist is final.
//
//  "Lazy deletion": std::priority_queue cannot lower a key, so when we find a
//  cheaper route we simply push the cell again. Old (stale) entries are skipped
//  when popped - that is what the `settled` check does.
// ============================================================================

#include "Solvers.h"
#include <algorithm>   // std::max
#include <chrono>
#include <functional>   // std::greater
#include <limits>
#include <queue>
#include <tuple>

SolverResult solveDijkstra(Maze maze) {
    SolverResult result;
    result.algorithm     = "Dijkstra";
    result.dataStructure = "Min-priority queue (by g)";

    auto t0 = std::chrono::steady_clock::now();

    const int n   = maze.cellCount();
    const int INF = std::numeric_limits<int>::max();
    std::vector<int>  dist(n, INF);
    std::vector<int>  parent(n, -1);
    std::vector<bool> settled(n, false);

    // Entry = (cost, insertion counter, cell). The counter breaks ties in FIFO
    // order, so results are repeatable. std::greater turns the max-heap into a min-heap.
    using Entry = std::tuple<int, int, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
    int counter = 0;

    const int startIdx = maze.toIndex(maze.getStart());
    const int endIdx   = maze.toIndex(maze.getEnd());

    dist[startIdx] = 0;
    pq.push({0, counter++, startIdx});
    result.events.push_back({EventType::Push, maze.getStart(), 0});
    bool reachedEnd = false;

    while (!pq.empty()) {
        result.maxFrontier = std::max(result.maxFrontier, static_cast<int>(pq.size()));
        auto [d, order, cur] = pq.top();
        pq.pop();
        (void)order;

        if (settled[cur]) {           // stale entry: a cheaper copy was already processed
            result.events.push_back({EventType::Skip, maze.fromIndex(cur), static_cast<double>(d)});
            continue;
        }
        settled[cur] = true;

        Position p = maze.fromIndex(cur);
        result.visitedOrder.push_back(p);
        result.events.push_back({EventType::Expand, p, static_cast<double>(d)});

        if (cur == endIdx) { reachedEnd = true; break; }

        for (const Position& nb : maze.getNeighbors(p)) {
            int ni = maze.toIndex(nb);
            if (settled[ni]) continue;
            int newDist = d + maze.getWeight(nb);   // cost of stepping ONTO nb
            if (newDist < dist[ni]) {               // "relaxation": found a cheaper route
                dist[ni]   = newDist;
                parent[ni] = cur;
                pq.push({newDist, counter++, ni});
                result.events.push_back({EventType::Push, nb, static_cast<double>(newDist)});
            }
        }
    }

    finishResult(maze, parent, reachedEnd, result);
    auto t1 = std::chrono::steady_clock::now();
    result.elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}
