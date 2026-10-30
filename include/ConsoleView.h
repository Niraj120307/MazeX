// ============================================================================
//  ConsoleView.h  -  Terminal "renderer" for search results
//
//  Search state (visited cells, final path) is NOT stored in the Maze. It is an
//  overlay drawn on top of it from a SolverResult. The SFML renderer (V0.6+)
//  will follow exactly the same rule.
// ============================================================================

#pragma once

#include <vector>
#include "Maze.h"
#include "SolverResult.h"

void setupConsole();                                   // UTF-8 + ANSI colours on Windows
void printSearch(const Maze& maze, const SolverResult& result);   // maze + visited + path overlay
void printSummary(const SolverResult& result);         // one result, as text
void printComparison(const std::vector<SolverResult>& results);   // side-by-side table
