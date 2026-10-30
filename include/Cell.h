// ============================================================================
//  Cell.h  -  The smallest building blocks of MAZE-X
//  Contains: CellType (what a square is), Position (where a square is),
//            Cell (the data stored in one square of the grid)
// ============================================================================

#pragma once   // Tells the compiler: "include this file only once", avoids duplicate-definition errors.

// ---------------------------------------------------------------------------
//  CellType: the possible states of ONE square in the maze.
//  "enum class" is a type-safe enum: you must write CellType::Wall (not just Wall),
//  so names can never clash with other code.
// ---------------------------------------------------------------------------
enum class CellType {
    Empty,   // a free square that a solver can walk through
    Wall     // a blocked square that a solver can NOT enter
};

// ---------------------------------------------------------------------------
//  Position: a coordinate (row, col) inside the grid.
//  We use a struct because it is a tiny bundle of two related numbers.
//  Later, every solver will put Positions into its Queue / Stack / Priority Queue.
// ---------------------------------------------------------------------------
struct Position {
    int row;   // vertical index   (0 = top row)
    int col;   // horizontal index (0 = leftmost column)

    // Constructor with default values, so both Position() and Position(3, 4) work.
    // ": row(r), col(c)" is the "initializer list" - the fastest way to set members.
    Position(int r = 0, int c = 0) : row(r), col(c) {}

    // Operator overloading: lets us write  a == b  to compare two positions.
    // "const" after the parentheses promises this function does not modify the object.
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;   // equal only if BOTH match
    }

    // Written using == so the logic exists in only one place.
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// ---------------------------------------------------------------------------
//  Cell: the data stored in every square of the grid.
//  Only the MAZE'S own facts live here. Solver facts (visited, parent, distance)
//  will live inside the solvers in Stage 2, keeping the Core Engine clean.
// ---------------------------------------------------------------------------
struct Cell {
    CellType type;   // Empty or Wall
    int weight;      // cost to step onto this cell (1 = normal). Used by Dijkstra/A* in Stage 3.

    // Every new cell starts as an empty, normal-cost square.
    Cell() : type(CellType::Empty), weight(1) {}
};