// ============================================================================
//  MazeGenerator.h  -  Automatic maze creation (V0.5)
//
//  All generators take a SEED: the same seed always produces the same maze,
//  so a demo or a benchmark can be repeated exactly.
// ============================================================================

#pragma once

#include "Maze.h"

// Recursive backtracking ("randomized DFS") maze.
//  - Cells with EVEN row and EVEN column are "rooms"; everything else starts as wall.
//  - Starting from the top-left room, repeatedly jump to a random unvisited room
//    2 cells away and knock down the wall between. When stuck, BACKTRACK (pop the
//    stack) until a room with unvisited neighbours is found.
//  - The result is a "perfect" maze: exactly one route between any two rooms.
//  - loopFraction (0.0 - 1.0) then removes that share of the remaining inner walls
//    between rooms, creating alternative routes (makes BFS vs DFS more interesting).
// Start = top-left room, End = bottom-right room. Always solvable.
void generateBacktracker(Maze& maze, unsigned seed, double loopFraction = 0.0);

// Scatter walls randomly with the given density (0.0 - 0.6). Retries with new
// random layouts until the End is reachable, so the result is always solvable.
// Keeps the current Start and End.
void generateRandomObstacles(Maze& maze, unsigned seed, double density = 0.28);

// Paint random "blobs" of expensive terrain (cost 2-9) over walkable cells.
// coverage = rough share of cells to cover (0.0 - 0.6). Walls are untouched.
void addRandomTerrain(Maze& maze, unsigned seed, double coverage = 0.3);

// Returns true when End can be reached from Start (simple BFS check).
bool isSolvable(const Maze& maze);
