// ============================================================================
//  Playback.h  -  Step-by-step replay of a finished search (V0.7 / V0.8)
//
//  The solver runs ONCE, at full speed, and records every Push / Expand / Skip.
//  Playback then replays those events one at a time for the animation. Because
//  of this split:
//    - animation speed can never change the result or the measured time
//    - pause / step / restart are trivial (just move an index)
//    - the live stack / queue / priority-queue panel is rebuilt from the real
//      events, so it shows the solver's ACTUAL data-structure contents.
//
//  No SFML here: Playback is tested in the console like the rest of the engine.
// ============================================================================

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "Maze.h"
#include "SolverResult.h"

enum class ContainerKind { Stack, Queue, PriorityQueue };

// How a cell should be coloured at the current moment of the replay.
enum class CellVisual { None, Frontier, Visited, Current, Path };

struct FrontierEntry {
    Position cell;
    double   priority;   // g for Dijkstra, f for A*, 0 for DFS/BFS
};

class Playback {
public:
    void load(const Maze& maze, const SolverResult& result);   // start a new replay at step 0
    void clear();                                              // remove all overlays
    void restart();                                            // back to step 0, same result

    bool step();          // apply ONE event; returns false if already finished
    void runToEnd();      // apply all remaining events

    bool loaded()   const { return loaded_; }
    bool finished() const { return loaded_ && index_ >= result_.events.size(); }
    std::size_t eventIndex() const { return index_; }
    std::size_t eventCount() const { return loaded_ ? result_.events.size() : 0; }
    int  exploredSoFar() const { return explored_; }
    bool hasCurrent() const { return hasCurrent_; }
    Position current() const { return current_; }
    const SolverResult& result() const { return result_; }
    ContainerKind kind() const { return kind_; }

    CellVisual visualAt(int index) const;                 // colour for flat cell index
    std::vector<FrontierEntry> displayOrder() const;      // stack: top first, queue: front first, PQ: smallest first
    std::size_t frontierSize() const { return container_.size(); }
    std::string lastAction() const { return lastAction_; }   // e.g. "pop (3,4)", "push (2,5) g=7"

    // Debug/test aid: how many pops did NOT come from the correct end of the
    // container (top of stack, front of queue, minimum of PQ). Must stay 0.
    int orderViolations() const { return violations_; }

    static ContainerKind kindFor(const std::string& algorithm);

private:
    void removeFor(const SearchEvent& e);

    bool loaded_ = false;
    SolverResult result_;
    int cols_ = 0;
    ContainerKind kind_ = ContainerKind::Queue;
    std::size_t index_ = 0;
    int explored_ = 0;
    bool hasCurrent_ = false;
    Position current_;
    std::vector<FrontierEntry> container_;   // insertion order (oldest first)
    std::vector<int> inFrontier_;            // how many copies of each cell are in the container
    std::vector<bool> visited_;
    std::vector<bool> onPath_;
    std::string lastAction_;
    int violations_ = 0;
};
