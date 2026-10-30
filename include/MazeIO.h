// ============================================================================
//  MazeIO.h  -  Build a maze from text, turn a maze back into text, save/load
//
//  Text format (one string per row, all rows the same length):
//      '#' wall      '.' empty (cost 1)      'S' start      'E' end
//      '2'..'9'      walkable terrain with that cost
//  Example:
//      "S..#"
//      ".#.."
//      "..5E"
//
//  Used by the tests (hand-made mazes are easy to read) and by save/load.
// ============================================================================

#pragma once

#include <string>
#include <vector>
#include "Maze.h"

// Throws std::invalid_argument for ragged rows, unknown characters, or a missing /
// duplicated S or E.
Maze mazeFromLines(const std::vector<std::string>& lines);

// Inverse of mazeFromLines: mazeFromLines(mazeToLines(m)) rebuilds the same maze.
std::vector<std::string> mazeToLines(const Maze& maze);

bool saveMaze(const Maze& maze, const std::string& filename);   // false if the file can't be written
Maze loadMaze(const std::string& filename);                     // throws on missing / invalid file
