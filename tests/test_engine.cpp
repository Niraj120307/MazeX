// ============================================================================
//  test_engine.cpp  -  Generator (V0.5), Playback (V0.7/V0.8), Benchmark (V0.9)
// ============================================================================

#include <cstdio>
#include <fstream>
#include <random>
#include "Benchmark.h"
#include "MazeGenerator.h"
#include "MazeIO.h"
#include "Playback.h"
#include "Solvers.h"
#include "TestUtil.h"

void runEngineTests() {
    section("Generator: recursive backtracking (V0.5)");
    {
        int allSolvable = 0, allRoomsReachable = 0, perfect = 0, total = 0;
        for (int size = 3; size <= 41; size += 2) {
            for (int rectangular = 0; rectangular < 2; ++rectangular) {
                int rows = size, cols = rectangular ? size + 6 : size;
                Maze m(rows, cols);
                generateBacktracker(m, 1000u + static_cast<unsigned>(size), 0.0);
                ++total;
                if (isSolvable(m)) ++allSolvable;

                // Every room (even, even) must be reachable from Start.
                SolverResult flood = solveBFS(m);   // BFS stops at End, so flood-fill separately:
                (void)flood;
                std::vector<bool> seen(m.cellCount(), false);
                std::vector<Position> stack = {m.getStart()};
                seen[m.toIndex(m.getStart())] = true;
                int openCells = 0;
                while (!stack.empty()) {
                    Position p = stack.back(); stack.pop_back(); ++openCells;
                    for (const Position& nb : m.getNeighbors(p))
                        if (!seen[m.toIndex(nb)]) { seen[m.toIndex(nb)] = true; stack.push_back(nb); }
                }
                bool rooms = true;
                int roomCount = 0;
                for (int r = 0; r < rows; r += 2)
                    for (int c = 0; c < cols; c += 2) { ++roomCount; if (!seen[m.toIndex(Position(r, c))]) rooms = false; }
                if (rooms) ++allRoomsReachable;
                // Perfect maze = spanning tree: open cells = rooms + (rooms - 1) connectors.
                if (openCells == 2 * roomCount - 1) ++perfect;
            }
        }
        std::string of = "/" + std::to_string(total);
        check(allSolvable == total,       "generated mazes are solvable " + std::to_string(allSolvable) + of);
        check(allRoomsReachable == total, "every room is connected " + std::to_string(allRoomsReachable) + of);
        check(perfect == total,           "loopFraction 0 gives a perfect maze (tree) " + std::to_string(perfect) + of);

        Maze a(21, 31), b(21, 31), c(21, 31);
        generateBacktracker(a, 7, 0.1);
        generateBacktracker(b, 7, 0.1);
        generateBacktracker(c, 8, 0.1);
        check(mazeToLines(a) == mazeToLines(b), "same seed -> identical maze");
        check(mazeToLines(a) != mazeToLines(c), "different seed -> different maze");

        Maze loopy(21, 21), tree(21, 21);
        generateBacktracker(tree, 5, 0.0);
        generateBacktracker(loopy, 5, 0.5);
        check(loopy.wallCount() < tree.wallCount(), "loopFraction removes extra walls");
        check(isSolvable(loopy), "maze with loops is still solvable");

        Maze tiny(2, 2);
        generateBacktracker(tiny, 1, 0.0);
        check(tiny.getStart() != tiny.getEnd() && isSolvable(tiny), "2x2 grid handled safely");
        Maze even(10, 12);
        generateBacktracker(even, 3, 0.0);
        check(isSolvable(even) && even.getEnd() == Position(8, 10), "even-sized grid: End on last room");
    }

    section("Generator: random obstacles and terrain");
    {
        int solvable = 0;
        for (unsigned s = 0; s < 100; ++s) {
            Maze m(15, 15);
            generateRandomObstacles(m, s, 0.4);
            if (isSolvable(m) && m.wallCount() > 0) ++solvable;
        }
        check(solvable == 100, "random obstacles always solvable and non-empty (100/100)");

        Maze t(25, 25);
        generateBacktracker(t, 9, 0.2);
        int wallsBefore = t.wallCount();
        addRandomTerrain(t, 9, 0.3);
        check(t.isWeighted(), "terrain adds weighted cells");
        check(t.wallCount() == wallsBefore, "terrain never changes walls");
        check(t.getWeight(t.getStart()) == 1 && t.getWeight(t.getEnd()) == 1, "Start/End keep cost 1");
        check(solveAStar(t).pathCost == solveDijkstra(t).pathCost, "A* == Dijkstra on generated terrain");
    }

    section("Playback replays the REAL data-structure state (V0.7/V0.8)");
    {
        std::mt19937 rng(77);
        int clean = 0, matches = 0, total = 0;
        for (int t = 0; t < 120; ++t) {
            Maze m(5 + static_cast<int>(rng() % 20), 5 + static_cast<int>(rng() % 20));
            if (t % 3 == 0) generateBacktracker(m, rng(), 0.2);
            else { m.setStartEnd(Position(0, 0), Position(m.getRows() - 1, m.getCols() - 1)); generateRandomObstacles(m, rng(), 0.3); }
            if (t % 2 == 0) addRandomTerrain(m, rng(), 0.3);

            for (const AlgorithmInfo& a : allAlgorithms()) {
                SolverResult r = a.solve(m);
                Playback pb;
                pb.load(m, r);
                std::size_t maxSize = 0;
                while (pb.step()) maxSize = std::max(maxSize, pb.frontierSize());
                ++total;
                if (pb.orderViolations() == 0) ++clean;
                if (pb.exploredSoFar() == r.nodesVisited && pb.finished() &&
                    static_cast<int>(maxSize) >= r.maxFrontier - 1) ++matches;
            }
        }
        std::string of = "/" + std::to_string(total);
        check(clean == total,   "every pop came from top/front/minimum " + std::to_string(clean) + of);
        check(matches == total, "replay explores exactly nodesVisited cells " + std::to_string(matches) + of);
    }
    {
        Maze m = mazeFromLines({"S..", "...", "..E"});
        SolverResult r = solveBFS(m);
        Playback pb;
        pb.load(m, r);
        check(!pb.finished() && pb.eventIndex() == 0, "starts at step 0");
        pb.step();   // push S
        check(pb.frontierSize() == 1 && pb.visualAt(0) == CellVisual::Frontier, "after push: S is in the queue");
        pb.step();   // pop S
        check(pb.frontierSize() == 0 && pb.hasCurrent() && pb.current() == m.getStart(), "after pop: S is current");
        pb.step(); pb.step();   // push right (0,1), push down (1,0)
        auto order = pb.displayOrder();
        check(order.size() == 2 && order[0].cell == Position(0, 1) && order[1].cell == Position(1, 0),
              "queue panel shows front first: (0,1) then (1,0)");
        check(pb.visualAt(m.toIndex(Position(2, 2))) == CellVisual::None, "untouched cell has no overlay");
        pb.runToEnd();
        check(pb.finished() && pb.visualAt(m.toIndex(m.getEnd())) == CellVisual::Path, "path shown when finished");
        pb.restart();
        check(pb.eventIndex() == 0 && pb.exploredSoFar() == 0 && pb.frontierSize() == 0, "restart rewinds everything");
        pb.clear();
        check(!pb.loaded() && pb.visualAt(0) == CellVisual::None, "clear removes overlays");

        Maze d = mazeFromLines({"S..", "...", "..E"});
        Playback st;
        st.load(d, solveDFS(d));
        st.step(); st.step(); st.step(); st.step();   // push S, pop S, push Down (1,0), push Right (0,1)
        auto top = st.displayOrder();
        check(st.kind() == ContainerKind::Stack && top.size() == 2 && top[0].cell == Position(0, 1),
              "stack panel shows the top first");
        Playback pq;
        pq.load(d, solveAStar(d));
        pq.runToEnd();
        auto pqOrder = pq.displayOrder();
        bool sorted = true;
        for (std::size_t i = 1; i < pqOrder.size(); ++i) if (pqOrder[i].priority < pqOrder[i - 1].priority) sorted = false;
        check(pq.kind() == ContainerKind::PriorityQueue && sorted, "priority-queue panel sorted smallest first");
    }

    section("Benchmark and comparison (V0.9 / V1.0)");
    {
        Maze m(31, 31);
        generateBacktracker(m, 11, 0.15);
        auto results = compareAll(m, 5);
        check(results.size() == 4, "compareAll runs all four algorithms");
        check(results[0].algorithm == "DFS" && results[3].algorithm == "A*", "in registry order");
        check(results[1].pathLength == results[2].pathLength && results[2].pathCost == results[3].pathCost,
              "BFS/Dijkstra/A* agree on an unweighted maze");
        bool timed = true;
        for (const auto& r : results) if (r.elapsedMs < 0) timed = false;
        check(timed, "median times recorded");

        auto rows = runBenchmarks({11, 21}, 3, 42);
        check(rows.size() == 2 * 3 * 4, "2 sizes x 3 maze types x 4 algorithms = 24 rows");
        const std::string file = "test_tmp_bench.csv";
        check(writeBenchmarkCsv(rows, file), "CSV written");
        std::ifstream in(file);
        std::string header, line;
        std::getline(in, header);
        int lines = 0;
        while (std::getline(in, line)) ++lines;
        check(header.rfind("size,maze_type,algorithm", 0) == 0 && lines == 24, "CSV has header + 24 rows");
        in.close();
        std::remove(file.c_str());
    }
}
