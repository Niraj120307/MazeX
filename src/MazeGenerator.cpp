// ============================================================================
//  MazeGenerator.cpp  -  Recursive backtracking, random obstacles, terrain
// ============================================================================

#include "MazeGenerator.h"
#include <algorithm>   // std::shuffle, std::clamp
#include <cstdlib>     // std::abs
#include <queue>
#include <random>
#include <stack>
#include <vector>

namespace {

// Largest even index <= n - 1 (the last "room" row/column).
int lastEven(int n) { return (n - 1) - ((n - 1) % 2); }

}  // namespace

bool isSolvable(const Maze& maze) {
    std::vector<bool> seen(maze.cellCount(), false);
    std::queue<Position> q;
    q.push(maze.getStart());
    seen[maze.toIndex(maze.getStart())] = true;
    while (!q.empty()) {
        Position p = q.front();
        q.pop();
        if (p == maze.getEnd()) return true;
        for (const Position& nb : maze.getNeighbors(p)) {
            int i = maze.toIndex(nb);
            if (!seen[i]) { seen[i] = true; q.push(nb); }
        }
    }
    return false;
}

void generateBacktracker(Maze& maze, unsigned seed, double loopFraction) {
    std::mt19937 rng(seed);
    const int rows = maze.getRows(), cols = maze.getCols();
    maze.clearWalls();
    maze.clearWeights();

    Position start(0, 0), end(lastEven(rows), lastEven(cols));
    if (start == end) {                       // grid too small for rooms (e.g. 2 x 2): leave it open
        maze.setStartEnd(Position(0, 0), Position(rows - 1, cols - 1));
        return;
    }
    maze.setStartEnd(start, end);

    // 1) Fill everything with walls (setWall refuses S and E, which are rooms anyway).
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            maze.setWall(Position(r, c));

    // 2) Carve with an explicit stack (iterative recursive-backtracking).
    std::vector<bool> visited(maze.cellCount(), false);
    std::stack<Position> st;
    st.push(start);
    visited[maze.toIndex(start)] = true;
    maze.removeWall(start);

    const int dr[4] = {-2, 0, 2, 0};
    const int dc[4] = {0, 2, 0, -2};

    while (!st.empty()) {
        Position cur = st.top();

        // Collect unvisited rooms two steps away.
        std::vector<int> options;
        for (int d = 0; d < 4; ++d) {
            Position next(cur.row + dr[d], cur.col + dc[d]);
            if (maze.isInside(next) && !visited[maze.toIndex(next)]) options.push_back(d);
        }

        if (options.empty()) {        // dead end -> backtrack
            st.pop();
            continue;
        }

        int d = options[rng() % options.size()];
        Position between(cur.row + dr[d] / 2, cur.col + dc[d] / 2);
        Position next(cur.row + dr[d], cur.col + dc[d]);
        maze.removeWall(between);     // knock down the wall between the two rooms
        maze.removeWall(next);
        visited[maze.toIndex(next)] = true;
        st.push(next);
    }

    // 3) Optional loops: open a share of the remaining walls that sit between two rooms.
    loopFraction = std::clamp(loopFraction, 0.0, 1.0);
    if (loopFraction > 0) {
        std::vector<Position> candidates;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                Position p(r, c);
                if (!maze.isWall(p)) continue;
                bool horizontal = (r % 2 == 0) && (c % 2 == 1) && c + 1 < cols;   // rooms left & right
                bool vertical   = (r % 2 == 1) && (c % 2 == 0) && r + 1 < rows;   // rooms above & below
                if (horizontal || vertical) candidates.push_back(p);
            }
        std::shuffle(candidates.begin(), candidates.end(), rng);
        std::size_t count = static_cast<std::size_t>(candidates.size() * loopFraction);
        for (std::size_t i = 0; i < count; ++i) maze.removeWall(candidates[i]);
    }
}

void generateRandomObstacles(Maze& maze, unsigned seed, double density) {
    std::mt19937 rng(seed);
    density = std::clamp(density, 0.0, 0.6);
    std::uniform_real_distribution<double> coin(0.0, 1.0);

    for (int attempt = 0; attempt < 200; ++attempt) {
        maze.clearWalls();
        for (int r = 0; r < maze.getRows(); ++r)
            for (int c = 0; c < maze.getCols(); ++c)
                if (coin(rng) < density) maze.setWall(Position(r, c));   // S/E are refused automatically
        if (isSolvable(maze)) return;
    }
    maze.clearWalls();   // extremely unlikely fallback: an open grid is always solvable
}

void addRandomTerrain(Maze& maze, unsigned seed, double coverage) {
    std::mt19937 rng(seed);
    coverage = std::clamp(coverage, 0.0, 0.6);
    maze.clearWeights();

    const int target = static_cast<int>(maze.cellCount() * coverage);
    int painted = 0, guard = 0;
    while (painted < target && guard++ < 10000) {
        Position centre(static_cast<int>(rng() % maze.getRows()), static_cast<int>(rng() % maze.getCols()));
        int radius = 1 + static_cast<int>(rng() % 3);          // blob size 1-3
        int weight = 3 + static_cast<int>(rng() % 7);          // cost 3-9
        for (int r = centre.row - radius; r <= centre.row + radius; ++r)
            for (int c = centre.col - radius; c <= centre.col + radius; ++c) {
                Position p(r, c);
                if (std::abs(r - centre.row) + std::abs(c - centre.col) > radius) continue;   // diamond
                if (!maze.isWalkable(p) || maze.getWeight(p) > 1) continue;
                if (p == maze.getStart() || p == maze.getEnd()) continue;
                maze.setWeight(p, weight);
                ++painted;
            }
    }
}
