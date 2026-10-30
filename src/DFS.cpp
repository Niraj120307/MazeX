// ============================================================================
//  DFS.cpp  -  Depth-First Search with an explicit STACK
//
//  Why an explicit std::stack instead of recursion?
//   - a 100 x 100 open maze can recurse 10,000 levels deep -> stack overflow risk
//   - the stack is the data structure we want to SHOW in the UI panel
//
//  Rule: a cell is marked visited when it is POPPED (expanded). A cell may sit
//  in the stack more than once; extra copies are skipped when popped. This gives
//  true depth-first order (always continue from the newest cell).
// ============================================================================

#include "Solvers.h"
#include <algorithm>   // std::max
#include <chrono>
#include <stack>
#include <utility>   // std::pair

SolverResult solveDFS(Maze maze) {
    SolverResult result;
    result.algorithm     = "DFS";
    result.dataStructure = "Stack";

    auto t0 = std::chrono::steady_clock::now();

    const int n = maze.cellCount();
    std::vector<bool> visited(n, false);
    std::vector<int>  parent(n, -1);

    // Each stack entry = (cell index, index of the cell that pushed it).
    std::stack<std::pair<int, int>> st;
    const int startIdx = maze.toIndex(maze.getStart());
    const int endIdx   = maze.toIndex(maze.getEnd());

    st.push({startIdx, -1});
    result.events.push_back({EventType::Push, maze.getStart(), 0});
    bool reachedEnd = false;

    while (!st.empty()) {
        result.maxFrontier = std::max(result.maxFrontier, static_cast<int>(st.size()));
        auto [cur, from] = st.top();   // LIFO: newest entry first
        st.pop();

        if (visited[cur]) {            // stale duplicate -> already explored
            result.events.push_back({EventType::Skip, maze.fromIndex(cur), 0});
            continue;
        }
        visited[cur] = true;
        parent[cur]  = from;

        Position p = maze.fromIndex(cur);
        result.visitedOrder.push_back(p);
        result.events.push_back({EventType::Expand, p, 0});

        if (cur == endIdx) { reachedEnd = true; break; }

        // Neighbours come back as Up, Right, Down, Left. Pushing them in REVERSE
        // means Up ends on top, so DFS still explores in Up, Right, Down, Left order.
        std::vector<Position> nbrs = maze.getNeighbors(p);
        for (auto it = nbrs.rbegin(); it != nbrs.rend(); ++it) {
            int ni = maze.toIndex(*it);
            if (!visited[ni]) {
                st.push({ni, cur});
                result.events.push_back({EventType::Push, *it, 0});
            }
        }
    }

    finishResult(maze, parent, reachedEnd, result);
    auto t1 = std::chrono::steady_clock::now();
    result.elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}
