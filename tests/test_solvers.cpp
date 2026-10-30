// ============================================================================
//  test_solvers.cpp  -  Tests for DFS, BFS, Dijkstra and A* (V0.2 - V0.4)
//
//  Hand-made mazes check the cases from the project plan (direct corridor,
//  forced detour, blocked destination, weighted alternative). Then hundreds of
//  random mazes are checked against an INDEPENDENT oracle (Bellman-Ford).
// ============================================================================

#include <functional>
#include <limits>
#include <random>
#include "MazeIO.h"
#include "Solvers.h"
#include "TestUtil.h"

namespace {

using TestSolver = std::function<SolverResult(Maze)>;
const std::vector<std::pair<std::string, TestSolver>> kSolvers = {
    {"DFS", solveDFS}, {"BFS", solveBFS}, {"Dijkstra", solveDijkstra}, {"A*", solveAStar},
};

// Bellman-Ford style oracle: keep relaxing every edge until nothing changes.
// Slow (O(V * E)) but completely different code, so it is a fair referee.
int oracleCost(const Maze& m) {
    const int INF = std::numeric_limits<int>::max() / 2;
    std::vector<int> dist(m.cellCount(), INF);
    dist[m.toIndex(m.getStart())] = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m.cellCount(); ++i) {
            Position p = m.fromIndex(i);
            if (dist[i] == INF || !m.isWalkable(p)) continue;
            for (const Position& nb : m.getNeighbors(p)) {
                int ni = m.toIndex(nb);
                if (dist[i] + m.getWeight(nb) < dist[ni]) { dist[ni] = dist[i] + m.getWeight(nb); changed = true; }
            }
        }
    }
    int d = dist[m.toIndex(m.getEnd())];
    return d == INF ? -1 : d;   // -1 = unreachable
}

// Fewest-moves oracle: the same relaxation with every weight treated as 1.
int oracleMoves(const Maze& m) {
    Maze copy = m;
    copy.clearWeights();
    return oracleCost(copy);
}

bool eventsConsistent(const SolverResult& r) {
    int expands = 0, size = 0;
    for (const SearchEvent& e : r.events) {
        if (e.type == EventType::Push) ++size;
        else { if (e.type == EventType::Expand) ++expands; --size; }   // Expand and Skip both pop
        if (size < 0) return false;   // can't take out more than was put in
    }
    return expands == r.nodesVisited;
}

}  // namespace

void runSolverTests() {
    section("Direct corridor");
    Maze corridor = mazeFromLines({
        "S...E",
        "#####",
    });
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(corridor);
        check(r.found && isValidPath(corridor, r.path) && r.pathLength == 4 && r.pathCost == 4,
              name + ": corridor path has 4 moves, cost 4");
        check(r.algorithm == name && !r.dataStructure.empty(), name + ": result labelled");
    }

    section("Wall forces a detour");
    Maze detour = mazeFromLines({
        "S.#..",
        "..#..",
        "..#.E",
        ".....",
    });
    // Shortest: down 3, right 4, up 1 = 8 moves.
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(detour);
        check(r.found && isValidPath(detour, r.path), name + ": legal detour found");
        if (name != "DFS") check(r.pathLength == 8, name + ": detour is the 8-move minimum");
    }

    section("Destination fully blocked (no path)");
    Maze blocked = mazeFromLines({
        "S....",
        "...##",
        "...#E",
    });
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(blocked);
        check(!r.found && r.path.empty() && r.pathLength == 0, name + ": reports no path, no crash");
        check(r.nodesVisited == 11, name + ": explored all 11 reachable cells before giving up");
    }

    section("Start walled in");
    Maze trapped = mazeFromLines({
        "S#...",
        "#....",
        "....E",
    });
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(trapped);
        check(!r.found && r.nodesVisited == 1, name + ": only the start is explored");
    }

    section("Adjacent start and end");
    Maze adjacent = mazeFromLines({"SE", ".."});
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(adjacent);
        check(r.found && r.pathLength == 1 && r.path.size() == 2, name + ": one move");
    }

    section("DFS is not always shortest (and is repeatable)");
    Maze open3 = mazeFromLines({
        "...",
        "...",
        "S.E",
    });
    SolverResult dfs = solveDFS(open3);
    SolverResult bfs = solveBFS(open3);
    check(bfs.pathLength == 2,                           "BFS: 2 moves (straight right)");
    check(dfs.found && isValidPath(open3, dfs.path) && dfs.pathLength == 6,
          "DFS: goes Up first and takes 6 moves");
    std::vector<Position> expected = {{2,0},{1,0},{0,0},{0,1},{0,2},{1,2},{2,2}};
    check(dfs.path == expected,                          "DFS path follows Up, Right, Down, Left order");
    SolverResult dfs2 = solveDFS(open3);
    check(dfs2.path == dfs.path && dfs2.visitedOrder == dfs.visitedOrder, "DFS gives identical result on re-run");

    section("BFS explores in rings");
    Maze open5(5, 5);
    open5.setStartEnd(Position(2, 2), Position(0, 0));
    SolverResult rings = solveBFS(open5);
    bool nonDecreasing = true;
    for (std::size_t i = 1; i < rings.visitedOrder.size(); ++i)
        if (manhattan(rings.visitedOrder[i], open5.getStart()) <
            manhattan(rings.visitedOrder[i - 1], open5.getStart())) nonDecreasing = false;
    check(nonDecreasing,                                 "BFS expands cells in non-decreasing distance");

    section("Weighted alternative (Dijkstra / A* avoid expensive terrain)");
    Maze swamp = mazeFromLines({
        "S999E",
        ".###.",
        ".....",
    });
    SolverResult wb = solveBFS(swamp), wd = solveDijkstra(swamp), wa = solveAStar(swamp);
    check(wb.pathLength == 4 && wb.pathCost == 28,       "BFS: 4 moves straight through the swamp, cost 28");
    check(wd.pathLength == 8 && wd.pathCost == 8,        "Dijkstra: 8 moves around the swamp, cost 8");
    check(wa.pathLength == 8 && wa.pathCost == 8,        "A*: same optimal cost as Dijkstra");

    Maze worthIt = mazeFromLines({
        "S2E",
        ".#.",
        "...",
    });
    check(solveDijkstra(worthIt).pathCost == 3,          "Dijkstra takes a cost-2 cell when the detour costs more");

    section("A* with heavy-everywhere terrain stays admissible");
    Maze heavy = mazeFromLines({
        "S3333",
        "33#33",
        "3333E",
    });
    heavy.setWeight(heavy.getStart(), 3);   // S and E default to cost 1; make every cell cost 3
    heavy.setWeight(heavy.getEnd(), 3);
    check(heavy.minWeight() == 3,                        "minWeight = 3 (heuristic scaled by 3)");
    check(solveAStar(heavy).pathCost == solveDijkstra(heavy).pathCost, "A* cost == Dijkstra cost");

    section("A* explores fewer cells than Dijkstra on an open grid");
    Maze big(30, 30);
    SolverResult bd = solveDijkstra(big), ba = solveAStar(big);
    check(ba.pathCost == bd.pathCost,                    "same cost (58)");
    check(ba.nodesVisited < bd.nodesVisited,             "A* explored " + std::to_string(ba.nodesVisited) +
                                                         " vs Dijkstra " + std::to_string(bd.nodesVisited));

    section("Results and event log are well-formed");
    for (auto& [name, solve] : kSolvers) {
        SolverResult r = solve(detour);
        check(r.visitedOrder.front() == detour.getStart(), name + ": first expanded cell is Start");
        check(r.visitedOrder.back() == detour.getEnd(),    name + ": stops as soon as End is expanded");
        check(r.nodesVisited == static_cast<int>(r.visitedOrder.size()), name + ": nodesVisited matches");
        check(eventsConsistent(r),                         name + ": push/expand log is consistent");
        check(r.maxFrontier >= 1 && r.elapsedMs >= 0,      name + ": frontier and time recorded");
    }
    Maze before = detour;
    solveDFS(detour); solveBFS(detour); solveDijkstra(detour); solveAStar(detour);
    check(mazeToLines(detour) == mazeToLines(before),    "solvers never modify the maze");

    section("Random mazes vs Bellman-Ford oracle (400 mazes)");
    std::mt19937 rng(12345);   // fixed seed -> the same 400 mazes every run
    int agreeFound = 0, bfsShortest = 0, dijkstraOptimal = 0, astarOptimal = 0,
        allValid = 0, dfsNotShorter = 0, total = 400, solvable = 0;
    for (int t = 0; t < total; ++t) {
        int rows = 2 + static_cast<int>(rng() % 14), cols = 2 + static_cast<int>(rng() % 14);
        Maze m(rows, cols);
        m.setStartEnd(Position(static_cast<int>(rng() % rows), static_cast<int>(rng() % cols)),
                      Position(static_cast<int>(rng() % rows), static_cast<int>(rng() % cols)));
        if (m.getStart() == m.getEnd()) m.setStartEnd(Position(0, 0), Position(rows - 1, cols - 1));
        bool weighted = (t % 2 == 1);
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                if (rng() % 100 < 28) m.setWall(Position(r, c));                       // ~28% walls
                else if (weighted) m.setWeight(Position(r, c), 1 + static_cast<int>(rng() % 9));
            }

        SolverResult d = solveDFS(m), b = solveBFS(m), dj = solveDijkstra(m), a = solveAStar(m);
        int cost = oracleCost(m), moves = oracleMoves(m);
        bool reachable = cost >= 0;
        if (reachable) ++solvable;

        if (d.found == reachable && b.found == reachable && dj.found == reachable && a.found == reachable)
            ++agreeFound;
        if (!reachable) { ++bfsShortest; ++dijkstraOptimal; ++astarOptimal; ++allValid; ++dfsNotShorter; continue; }
        if (isValidPath(m, d.path) && isValidPath(m, b.path) && isValidPath(m, dj.path) && isValidPath(m, a.path))
            ++allValid;
        if (b.pathLength == moves) ++bfsShortest;
        if (dj.pathCost == cost) ++dijkstraOptimal;
        if (a.pathCost == cost) ++astarOptimal;
        if (d.pathLength >= b.pathLength) ++dfsNotShorter;
    }
    std::string of = "/" + std::to_string(total);
    check(agreeFound == total,      "all solvers agree with oracle on reachability " + std::to_string(agreeFound) + of +
                                    " (" + std::to_string(solvable) + " solvable)");
    check(allValid == total,        "every returned path is legal " + std::to_string(allValid) + of);
    check(bfsShortest == total,     "BFS = fewest moves " + std::to_string(bfsShortest) + of);
    check(dijkstraOptimal == total, "Dijkstra = minimum cost " + std::to_string(dijkstraOptimal) + of);
    check(astarOptimal == total,    "A* = minimum cost " + std::to_string(astarOptimal) + of);
    check(dfsNotShorter == total,   "DFS never beats BFS on moves " + std::to_string(dfsNotShorter) + of);
}
