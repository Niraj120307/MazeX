// ============================================================================
//  SolverResult.cpp  -  Helpers shared by every solver
// ============================================================================

#include "SolverResult.h"
#include "Solvers.h"
#include <algorithm>   // std::reverse
#include <cstdlib>     // std::abs

std::vector<Position> reconstructPath(const Maze& maze, const std::vector<int>& parent) {
    std::vector<Position> path;
    int startIdx = maze.toIndex(maze.getStart());
    int cur      = maze.toIndex(maze.getEnd());

    // Follow the "who discovered me?" links backwards: End -> ... -> Start.
    while (cur != -1) {
        path.push_back(maze.fromIndex(cur));
        if (cur == startIdx) break;
        cur = parent[cur];
    }

    // If the walk did not reach the Start, the End was never discovered.
    if (path.empty() || path.back() != maze.getStart()) return {};

    std::reverse(path.begin(), path.end());   // now it reads Start -> End
    return path;
}

double computePathCost(const Maze& maze, const std::vector<Position>& path) {
    double cost = 0;
    for (std::size_t i = 1; i < path.size(); ++i)   // i = 1: we never "enter" the start
        cost += maze.getWeight(path[i]);
    return cost;
}

bool isValidPath(const Maze& maze, const std::vector<Position>& path) {
    if (path.empty()) return false;
    if (path.front() != maze.getStart() || path.back() != maze.getEnd()) return false;

    for (std::size_t i = 0; i < path.size(); ++i) {
        if (!maze.isWalkable(path[i])) return false;                 // wall or outside grid
        if (i > 0) {
            int dr = std::abs(path[i].row - path[i - 1].row);
            int dc = std::abs(path[i].col - path[i - 1].col);
            if (dr + dc != 1) return false;                          // must be one 4-direction step
        }
    }
    return true;
}

void finishResult(const Maze& maze, const std::vector<int>& parent, bool reachedEnd,
                  SolverResult& result) {
    result.nodesVisited = static_cast<int>(result.visitedOrder.size());
    if (!reachedEnd) {                         // "No path found" is a normal outcome, not a crash
        result.found = false;
        result.path.clear();
        result.pathLength = 0;
        result.pathCost = 0;
        return;
    }
    result.path       = reconstructPath(maze, parent);
    result.found      = !result.path.empty();
    result.pathLength = result.found ? static_cast<int>(result.path.size()) - 1 : 0;
    result.pathCost   = computePathCost(maze, result.path);
}

const std::vector<AlgorithmInfo>& allAlgorithms() {
    static const std::vector<AlgorithmInfo> list = {
        {"DFS", solveDFS, "Stack (LIFO)", "O(V + E)",
         "Goes deep first, then backtracks. Finds a path, not always the shortest."},
        {"BFS", solveBFS, "Queue (FIFO)", "O(V + E)",
         "Expands in rings from the start. Fewest moves; ignores terrain cost."},
        {"Dijkstra", solveDijkstra, "Min-priority queue by g", "O((V + E) log V)",
         "Always expands the cheapest known cell. Minimum total cost (weights >= 1)."},
        {"A*", solveAStar, "Min-priority queue by f = g + h", "O((V + E) log V)",
         "Dijkstra pulled toward the goal by h = Manhattan x min cost. Same optimal cost, fewer cells."},
    };
    return list;
}
