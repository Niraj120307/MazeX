// ============================================================================
//  test_maze.cpp  -  Tests for the Maze model and MazeIO (V0.1 + V0.4 terrain)
// ============================================================================

#include <cstdio>     // std::remove
#include <stdexcept>
#include "Maze.h"
#include "MazeIO.h"
#include "TestUtil.h"

void runMazeTests() {
    section("Maze: construction, walls, start/end (V0.1)");
    Maze maze(10, 20);
    maze.setStart(Position(1, 1));
    maze.setEnd(Position(8, 18));
    for (int r = 0; r <= 6; ++r) maze.setWall(Position(r, 6));
    for (int r = 3; r <= 9; ++r) maze.setWall(Position(r, 12));

    check(maze.getRows() == 10 && maze.getCols() == 20,  "size is 10 x 20");
    check(maze.cellCount() == 200,                       "cellCount is 200");
    check(maze.isInside(Position(0, 0)),                 "(0,0) is inside");
    check(!maze.isInside(Position(10, 0)),               "(10,0) is outside");
    check(!maze.isInside(Position(0, -1)),               "(0,-1) is outside");
    check(maze.isWall(Position(2, 6)),                   "(2,6) is a wall");
    check(maze.wallCount() == 14,                        "14 walls were built");
    check(!maze.isWall(Position(-5, -5)),                "outside cell is not reported as wall");
    check(!maze.isWalkable(Position(2, 6)),              "wall is not walkable");
    check(!maze.isWalkable(Position(99, 0)),             "outside cell is not walkable");
    check(!maze.setWall(Position(1, 1)),                 "cannot wall over Start");
    check(!maze.setWall(Position(8, 18)),                "cannot wall over End");
    check(!maze.setWall(Position(-1, 5)),                "cannot wall outside grid");
    check(!maze.setEnd(Position(1, 1)),                  "End cannot equal Start");
    check(!maze.setStart(Position(8, 18)),               "Start cannot equal End");
    check(!maze.setStart(Position(10, 10)),              "Start cannot be outside");
    check(maze.getNeighbors(Position(2, 5)).size() == 3, "(2,5) has 3 neighbors (right is a wall)");
    check(maze.getNeighbors(Position(0, 0)).size() == 2, "corner (0,0) has 2 neighbors");

    // Neighbour ORDER must be Up, Right, Down, Left (repeatable runs).
    auto nb = maze.getNeighbors(Position(5, 3));
    check(nb.size() == 4 && nb[0] == Position(4, 3) && nb[1] == Position(5, 4) &&
          nb[2] == Position(6, 3) && nb[3] == Position(5, 2), "neighbour order is Up, Right, Down, Left");

    maze.toggleWall(Position(2, 6));
    check(!maze.isWall(Position(2, 6)),                  "toggle removes a wall");
    check(maze.getNeighbors(Position(2, 5)).size() == 4, "(2,5) now has 4 neighbors");
    check(maze.toggleWall(Position(2, 6)) && maze.isWall(Position(2, 6)), "toggle builds it again");

    // Moving Start onto a wall clears that wall (Start can never be a wall).
    check(maze.setStart(Position(2, 6)) && !maze.isWall(Position(2, 6)), "setStart on a wall clears it");

    maze.clearWalls();
    check(maze.wallCount() == 0,                         "clearWalls removes all walls");
    check(maze.getStart() == Position(2, 6),             "clearWalls keeps Start");

    bool caught = false;
    try { Maze bad(1, 1); } catch (const std::invalid_argument&) { caught = true; }
    check(caught,                                        "1x1 maze throws");
    caught = false;
    try { Maze bad(1, 50); } catch (const std::invalid_argument&) { caught = true; }
    check(caught,                                        "1x50 maze throws");

    section("Maze: indexing and start/end swap");
    Maze m(4, 7);
    bool roundTrip = true;
    for (int i = 0; i < m.cellCount(); ++i)
        if (m.toIndex(m.fromIndex(i)) != i) roundTrip = false;
    check(roundTrip,                                     "toIndex(fromIndex(i)) == i for every cell");
    check(m.toIndex(Position(2, 3)) == 17,               "(2,3) -> 2*7+3 = 17");
    check(m.setStartEnd(Position(3, 6), Position(0, 0)), "setStartEnd can swap the corners");
    check(m.getStart() == Position(3, 6) && m.getEnd() == Position(0, 0), "swap applied");
    check(!m.setStartEnd(Position(1, 1), Position(1, 1)), "setStartEnd rejects S == E");

    section("Maze: weighted terrain (V0.4)");
    check(!m.isWeighted() && m.minWeight() == 1,         "new maze is unweighted, min cost 1");
    check(m.setWeight(Position(1, 1), 5),                "set weight 5");
    check(m.getWeight(Position(1, 1)) == 5,              "weight reads back as 5");
    check(m.isWeighted(),                                "maze is now weighted");
    check(!m.setWeight(Position(1, 1), 0),               "weight 0 rejected");
    check(!m.setWeight(Position(1, 1), -3),              "negative weight rejected");
    check(!m.setWeight(Position(1, 1), 10),              "weight 10 rejected");
    check(!m.setWeight(Position(9, 9), 2),               "weight outside grid rejected");
    m.clearWeights();
    check(!m.isWeighted(),                               "clearWeights resets to cost 1");
    Maze allHeavy(2, 2);
    for (int r = 0; r < 2; ++r) for (int c = 0; c < 2; ++c) allHeavy.setWeight(Position(r, c), 3);
    check(allHeavy.minWeight() == 3,                     "minWeight finds the cheapest cell");

    section("MazeIO: text format, save/load");
    std::vector<std::string> text = {
        "S..#5",
        ".#2..",
        "...#E",
    };
    Maze t = mazeFromLines(text);
    check(t.getStart() == Position(0, 0) && t.getEnd() == Position(2, 4), "S and E parsed");
    check(t.isWall(Position(0, 3)) && t.isWall(Position(1, 1)), "walls parsed");
    check(t.getWeight(Position(0, 4)) == 5 && t.getWeight(Position(1, 2)) == 2, "weights parsed");
    check(mazeToLines(t) == text,                        "mazeToLines(mazeFromLines(x)) == x");

    Maze swapped = mazeFromLines({"E.", ".S"});
    check(swapped.getStart() == Position(1, 1) && swapped.getEnd() == Position(0, 0),
          "S/E on opposite default corners parse correctly");

    const std::string file = "test_tmp_maze.txt";
    check(saveMaze(t, file),                             "saveMaze writes a file");
    Maze loaded = loadMaze(file);
    check(mazeToLines(loaded) == text,                   "loadMaze restores the same maze");
    std::remove(file.c_str());

    auto throws = [](const std::vector<std::string>& lines) {
        try { mazeFromLines(lines); } catch (const std::invalid_argument&) { return true; }
        return false;
    };
    check(throws({"S..", "..E", "."}),                   "ragged rows rejected");
    check(throws({"S.x", "..E"}),                        "unknown character rejected");
    check(throws({"S..", "..."}),                        "missing E rejected");
    check(throws({"S.S", "..E"}),                        "two S rejected");
    check(throws({}),                                    "empty text rejected");
    bool missing = false;
    try { loadMaze("does_not_exist_123.txt"); } catch (const std::invalid_argument&) { missing = true; }
    check(missing,                                       "loading a missing file throws");
}
