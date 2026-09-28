// ============================================================================
//  Maze.cpp  -  Implementation (the "kitchen") of the Maze class
// ============================================================================

#include "Maze.h"
#include <iostream>     // std::cout for printing
#include <stdexcept>    // std::invalid_argument for bad constructor input

// ANSI escape codes: special text that tells the terminal to change colors.
// "\033[" starts the code, the number picks the color, "m" ends it.
namespace {   // anonymous namespace = these names are private to this file
    const char* RESET  = "\033[0m";    // back to normal color
    const char* GREEN  = "\033[92m";   // bright green  -> Start
    const char* RED    = "\033[91m";   // bright red    -> End
    const char* GRAY   = "\033[90m";   // dark gray     -> Walls
    const char* CYAN   = "\033[96m";   // bright cyan   -> Frame
}

// ---------------------------------------------------------------------------
//  Constructor: build an empty grid, place default start (top-left) and end (bottom-right)
// ---------------------------------------------------------------------------
Maze::Maze(int rows, int cols)
    : rows_(rows), cols_(cols),
      grid_(rows > 0 ? rows : 0, std::vector<Cell>(cols > 0 ? cols : 0)),  // rows x cols of default Cells
      start_(0, 0),
      end_(rows - 1, cols - 1)
{
    // A maze needs at least 2 squares so Start and End can be different.
    if (rows < 2 || cols < 2) {
        throw std::invalid_argument("Maze must be at least 2 x 2");   // stop with a clear error
    }
}

// ---------------------------------------------------------------------------
//  Size & bounds
// ---------------------------------------------------------------------------
int Maze::getRows() const { return rows_; }
int Maze::getCols() const { return cols_; }

// Inside means: 0 <= row < rows_  AND  0 <= col < cols_.
// EVERY other function calls this first, so we never read outside the vector (a crash).
bool Maze::isInside(const Position& p) const {
    return p.row >= 0 && p.row < rows_ &&
           p.col >= 0 && p.col < cols_;
}

// ---------------------------------------------------------------------------
//  Wall system
// ---------------------------------------------------------------------------
bool Maze::isWall(const Position& p) const {
    if (!isInside(p)) return false;                        // outside the grid: not a wall cell
    return grid_[p.row][p.col].type == CellType::Wall;     // read the cell's type
}

bool Maze::setWall(const Position& p) {
    if (!isInside(p)) return false;                        // rule 1: must be on the grid
    if (p == start_ || p == end_) return false;            // rule 2: cannot wall over Start or End
    grid_[p.row][p.col].type = CellType::Wall;             // all rules passed -> build the wall
    return true;                                           // tell the caller it worked
}

bool Maze::removeWall(const Position& p) {
    if (!isInside(p)) return false;
    grid_[p.row][p.col].type = CellType::Empty;            // erasing is always safe
    return true;
}

bool Maze::toggleWall(const Position& p) {
    if (!isInside(p)) return false;
    // If it is a wall, remove it; otherwise try to build one. One click = one flip.
    return isWall(p) ? removeWall(p) : setWall(p);
}

void Maze::clearWalls() {
    for (auto& row : grid_)          // "auto&" = reference to each row (no copying)
        for (auto& cell : row)       // visit every cell in that row
            cell.type = CellType::Empty;
}

// ---------------------------------------------------------------------------
//  Start / End
// ---------------------------------------------------------------------------
bool Maze::setStart(const Position& p) {
    if (!isInside(p) || p == end_) return false;           // must be on grid and different from End
    start_ = p;                                            // move the start
    grid_[p.row][p.col].type = CellType::Empty;            // if a wall was here, remove it
    return true;
}

bool Maze::setEnd(const Position& p) {
    if (!isInside(p) || p == start_) return false;         // must be on grid and different from Start
    end_ = p;
    grid_[p.row][p.col].type = CellType::Empty;
    return true;
}

Position Maze::getStart() const { return start_; }
Position Maze::getEnd()   const { return end_; }

// ---------------------------------------------------------------------------
//  Graph view: the bridge between "grid" and "graph"
//  Each walkable cell is a VERTEX; each pair of adjacent walkable cells is an EDGE.
//  This function returns the edges leaving p, so BFS/DFS/Dijkstra/A* never
//  need to know how the grid works internally.
// ---------------------------------------------------------------------------
std::vector<Position> Maze::getNeighbors(const Position& p) const {
    std::vector<Position> result;                          // the answer list

    // Fixed order: Up, Right, Down, Left. A fixed order makes runs repeatable,
    // which is important for step-by-step demos and testing.
    const int dRow[4] = {-1, 0, 1, 0};                     // row change for each direction
    const int dCol[4] = { 0, 1, 0, -1};                    // column change for each direction

    for (int i = 0; i < 4; ++i) {
        Position next(p.row + dRow[i], p.col + dCol[i]);   // candidate neighbor
        if (isInside(next) && !isWall(next)) {             // keep only legal, walkable squares
            result.push_back(next);
        }
    }
    return result;
}

int Maze::getWeight(const Position& p) const {
    if (!isInside(p)) return 0;
    return grid_[p.row][p.col].weight;
}

// ---------------------------------------------------------------------------
//  Console display: each cell is 2 characters wide so squares look square
// ---------------------------------------------------------------------------
void Maze::print() const {
    // Top border: "╔" + "══" for every column + "╗"
    std::cout << CYAN << "╔";
    for (int c = 0; c < cols_; ++c) std::cout << "══";
    std::cout << "╗" << RESET << "\n";

    for (int r = 0; r < rows_; ++r) {
        std::cout << CYAN << "║" << RESET;                 // left border
        for (int c = 0; c < cols_; ++c) {
            Position p(r, c);
            if (p == start_)      std::cout << GREEN << "S " << RESET;    // start
            else if (p == end_)   std::cout << RED   << "E " << RESET;    // end
            else if (isWall(p))   std::cout << GRAY  << "██" << RESET;    // wall
            else                  std::cout << "· ";                      // free cell
        }
        std::cout << CYAN << "║" << RESET << "\n";         // right border
    }

    // Bottom border
    std::cout << CYAN << "╚";
    for (int c = 0; c < cols_; ++c) std::cout << "══";
    std::cout << "╝" << RESET << "\n";
}