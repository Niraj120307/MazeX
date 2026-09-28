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
    // ---- Construction -------------------------------------------------------
    Maze(int rows, int cols);            // builds an empty rows x cols maze

    // ---- Size & bounds ------------------------------------------------------
    int  getRows() const;                // number of rows
    int  getCols() const;                // number of columns
    bool isInside(const Position& p) const;   // is p within the grid?

    // ---- Wall system --------------------------------------------------------
    bool isWall(const Position& p) const;     // is this square a wall?
    bool setWall(const Position& p);          // build a wall   (false if not allowed)
    bool removeWall(const Position& p);       // erase a wall   (false if out of bounds)
    bool toggleWall(const Position& p);       // wall <-> empty (handy for mouse clicks)
    void clearWalls();                        // erase ALL walls, keep start & end

    // ---- Start / End --------------------------------------------------------
    bool     setStart(const Position& p);     // move the start point
    bool     setEnd(const Position& p);       // move the end point
    Position getStart() const;
    Position getEnd() const;

    // ---- Graph view (used by every solver in Stage 2+) ----------------------
    std::vector<Position> getNeighbors(const Position& p) const;  // walkable adjacent squares
    int getWeight(const Position& p) const;                       // cost of entering p

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