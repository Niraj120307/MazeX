// ============================================================================
//  Maze.h  -  Declaration (the "menu") of the Maze class
//  The Maze owns the grid, the start, the end, and every wall rule.
//  It knows NOTHING about SFML or algorithms -> the Core Engine stays independent.
// ============================================================================

#pragma once

#include <vector>      // std::vector : dynamic array, used for the grid and neighbor lists
#include "Cell.h"      // our Position / Cell / CellType

class Maze {
public:
    // Allowed terrain cost range. 1 = normal ground, 9 = very expensive (mud, water...).
    static constexpr int kMinWeight = 1;
    static constexpr int kMaxWeight = 9;

    // ---- Construction -------------------------------------------------------
    Maze(int rows, int cols);            // builds an empty rows x cols maze

    // ---- Size & bounds ------------------------------------------------------
    int  getRows() const;                // number of rows
    int  getCols() const;                // number of columns
    int  cellCount() const;              // rows * cols (size of solver arrays)
    bool isInside(const Position& p) const;   // is p within the grid?

    // ---- Flat indexing (used by solvers for visited[] / parent[] arrays) -----
    // A 2D cell (r, c) maps to ONE number: r * cols + c. Arrays indexed this way
    // are faster and simpler than maps keyed by Position.
    int      toIndex(const Position& p) const;
    Position fromIndex(int index) const;

    // ---- Wall system --------------------------------------------------------
    bool isWall(const Position& p) const;     // is this square a wall?
    bool isWalkable(const Position& p) const; // inside the grid AND not a wall
    bool setWall(const Position& p);          // build a wall   (false if not allowed)
    bool removeWall(const Position& p);       // erase a wall   (false if out of bounds)
    bool toggleWall(const Position& p);       // wall <-> empty (handy for mouse clicks)
    void clearWalls();                        // erase ALL walls, keep start & end
    int  wallCount() const;                   // how many walls exist

    // ---- Start / End --------------------------------------------------------
    bool     setStart(const Position& p);     // move the start point
    bool     setEnd(const Position& p);       // move the end point
    bool     setStartEnd(const Position& s, const Position& e);  // move both at once (e.g. swap them)
    Position getStart() const;
    Position getEnd() const;

    // ---- Weighted terrain (V0.4: used by Dijkstra and A*) -------------------
    bool setWeight(const Position& p, int weight);  // false if outside or weight not in [1, 9]
    int  getWeight(const Position& p) const;        // cost of ENTERING p (0 if outside)
    void clearWeights();                            // every cell back to cost 1
    int  minWeight() const;                         // cheapest walkable cell (for A* heuristic)
    bool isWeighted() const;                        // true if any walkable cell costs more than 1

    // ---- Graph view (used by every solver) ----------------------------------
    std::vector<Position> getNeighbors(const Position& p) const;  // walkable adjacent squares

    // ---- Console display ----------------------------------------------------
    void print() const;                       // draws the maze in the terminal

private:
    // Private = only Maze's own functions can touch these. This protects the rules
    // (e.g. nobody can put a wall on the start square by editing the grid directly).
    int rows_;                                // trailing underscore = "member variable" naming style
    int cols_;
    std::vector<std::vector<Cell>> grid_;     // 2D grid: grid_[row][col]
    Position start_;                          // where the search begins
    Position end_;                            // where the search must reach
};
