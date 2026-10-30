// ============================================================================
//  Playback.cpp  -  Replays SearchEvents and rebuilds the live data structure
// ============================================================================

#include "Playback.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace {

std::string cellText(const Position& p) {
    return "(" + std::to_string(p.row) + "," + std::to_string(p.col) + ")";
}

std::string priorityText(ContainerKind kind, const std::string& algorithm, double priority) {
    if (kind != ContainerKind::PriorityQueue) return "";
    std::ostringstream out;
    out << (algorithm == "A*" ? " f=" : " g=") << priority;
    return out.str();
}

bool samePriority(double a, double b) { return std::fabs(a - b) < 1e-9; }

}  // namespace

ContainerKind Playback::kindFor(const std::string& algorithm) {
    if (algorithm == "DFS") return ContainerKind::Stack;
    if (algorithm == "BFS") return ContainerKind::Queue;
    return ContainerKind::PriorityQueue;
}

void Playback::load(const Maze& maze, const SolverResult& result) {
    result_ = result;
    cols_   = maze.getCols();
    kind_   = kindFor(result.algorithm);
    loaded_ = true;

    onPath_.assign(maze.cellCount(), false);
    for (const Position& p : result_.path) onPath_[maze.toIndex(p)] = true;
    inFrontier_.assign(maze.cellCount(), 0);
    visited_.assign(maze.cellCount(), false);
    restart();
}

void Playback::clear() {
    loaded_ = false;
    result_ = SolverResult();
    container_.clear();
    inFrontier_.clear();
    visited_.clear();
    onPath_.clear();
    index_ = 0;
    explored_ = 0;
    hasCurrent_ = false;
    lastAction_.clear();
    violations_ = 0;
}

void Playback::restart() {
    index_ = 0;
    explored_ = 0;
    hasCurrent_ = false;
    violations_ = 0;
    container_.clear();
    std::fill(inFrontier_.begin(), inFrontier_.end(), 0);
    std::fill(visited_.begin(), visited_.end(), false);
    lastAction_ = "ready";
}

// Take the matching entry out of the container, from the end the real data
// structure would use, and count it as a violation if it was not there.
void Playback::removeFor(const SearchEvent& e) {
    if (container_.empty()) { ++violations_; return; }
    int found = -1;

    if (kind_ == ContainerKind::Stack) {                       // LIFO: newest entry
        for (int i = static_cast<int>(container_.size()) - 1; i >= 0; --i)
            if (container_[i].cell == e.cell) { found = i; break; }
        if (found != static_cast<int>(container_.size()) - 1) ++violations_;
    } else if (kind_ == ContainerKind::Queue) {                // FIFO: oldest entry
        for (int i = 0; i < static_cast<int>(container_.size()); ++i)
            if (container_[i].cell == e.cell) { found = i; break; }
        if (found != 0) ++violations_;
    } else {                                                   // PQ: smallest priority
        double minPriority = container_[0].priority;
        for (const FrontierEntry& f : container_) minPriority = std::min(minPriority, f.priority);
        for (int i = 0; i < static_cast<int>(container_.size()); ++i)
            if (container_[i].cell == e.cell && samePriority(container_[i].priority, e.priority)) { found = i; break; }
        if (found < 0 || !samePriority(container_[found].priority, minPriority)) ++violations_;
    }

    if (found < 0) return;
    int idx = e.cell.row * cols_ + e.cell.col;
    --inFrontier_[idx];
    container_.erase(container_.begin() + found);
}

bool Playback::step() {
    if (!loaded_ || finished()) return false;
    const SearchEvent& e = result_.events[index_++];
    const int idx = e.cell.row * cols_ + e.cell.col;
    const std::string pr = priorityText(kind_, result_.algorithm, e.priority);

    switch (e.type) {
        case EventType::Push:
            container_.push_back({e.cell, e.priority});
            ++inFrontier_[idx];
            lastAction_ = "push " + cellText(e.cell) + pr;
            break;
        case EventType::Expand:
            removeFor(e);
            visited_[idx] = true;
            ++explored_;
            current_ = e.cell;
            hasCurrent_ = true;
            lastAction_ = "pop " + cellText(e.cell) + pr + "  -> explore";
            break;
        case EventType::Skip:
            removeFor(e);
            lastAction_ = "pop " + cellText(e.cell) + pr + "  -> stale, skip";
            break;
    }
    if (finished()) {
        hasCurrent_ = false;
        lastAction_ = result_.found ? "done: path found" : "done: no path";
    }
    return true;
}

void Playback::runToEnd() {
    while (step()) {}
}

CellVisual Playback::visualAt(int index) const {
    if (!loaded_ || index < 0 || index >= static_cast<int>(visited_.size())) return CellVisual::None;
    if (finished() && onPath_[index]) return CellVisual::Path;
    if (hasCurrent_ && current_.row * cols_ + current_.col == index) return CellVisual::Current;
    if (inFrontier_[index] > 0) return CellVisual::Frontier;
    if (visited_[index]) return CellVisual::Visited;
    return CellVisual::None;
}

std::vector<FrontierEntry> Playback::displayOrder() const {
    std::vector<FrontierEntry> out = container_;
    if (kind_ == ContainerKind::Stack) {
        std::reverse(out.begin(), out.end());                   // top of stack first
    } else if (kind_ == ContainerKind::PriorityQueue) {
        std::stable_sort(out.begin(), out.end(),                 // smallest priority first
                         [](const FrontierEntry& a, const FrontierEntry& b) { return a.priority < b.priority; });
    }                                                            // queue: already front first
    return out;
}
