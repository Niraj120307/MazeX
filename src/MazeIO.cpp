// ============================================================================
//  MazeIO.cpp  -  Text <-> Maze conversion and file save/load
// ============================================================================

#include "MazeIO.h"
#include <fstream>
#include <stdexcept>

Maze mazeFromLines(const std::vector<std::string>& lines) {
    if (lines.empty()) throw std::invalid_argument("maze text is empty");
    const int rows = static_cast<int>(lines.size());
    const int cols = static_cast<int>(lines[0].size());

    Maze maze(rows, cols);   // throws if smaller than 2 x 2

    int startCount = 0, endCount = 0;
    Position start, end;

    for (int r = 0; r < rows; ++r) {
        if (static_cast<int>(lines[r].size()) != cols)
            throw std::invalid_argument("row " + std::to_string(r) + " has a different length");
        for (int c = 0; c < cols; ++c) {
            char ch = lines[r][c];
            if      (ch == 'S') { start = Position(r, c); ++startCount; }
            else if (ch == 'E') { end   = Position(r, c); ++endCount; }
            else if (ch == '.' || ch == '#') { /* handled below */ }
            else if (ch >= '2' && ch <= '9') { /* handled below */ }
            else throw std::invalid_argument(std::string("unknown maze character '") + ch + "'");
        }
    }
    if (startCount != 1 || endCount != 1)
        throw std::invalid_argument("maze needs exactly one S and one E");

    maze.setStartEnd(start, end);   // both at once, so the default corners never get in the way

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            char ch = lines[r][c];
            if (ch == '#') maze.setWall(Position(r, c));
            else if (ch >= '2' && ch <= '9') maze.setWeight(Position(r, c), ch - '0');
        }
    return maze;
}

std::vector<std::string> mazeToLines(const Maze& maze) {
    std::vector<std::string> lines;
    for (int r = 0; r < maze.getRows(); ++r) {
        std::string row;
        for (int c = 0; c < maze.getCols(); ++c) {
            Position p(r, c);
            if      (p == maze.getStart())  row += 'S';
            else if (p == maze.getEnd())    row += 'E';
            else if (maze.isWall(p))        row += '#';
            else if (maze.getWeight(p) > 1) row += static_cast<char>('0' + maze.getWeight(p));
            else                            row += '.';
        }
        lines.push_back(row);
    }
    return lines;
}

bool saveMaze(const Maze& maze, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) return false;
    for (const std::string& line : mazeToLines(maze)) out << line << '\n';
    return static_cast<bool>(out);
}

Maze loadMaze(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::invalid_argument("cannot open " + filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();   // Windows line endings
        if (!line.empty()) lines.push_back(line);
    }
    return mazeFromLines(lines);
}
