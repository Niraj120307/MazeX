// ============================================================================
//  Renderer.cpp  -  Everything the window draws. No algorithm logic lives here:
//  it only reads the Maze, the Playback state and the SolverResults.
// ============================================================================

#include "App.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "../../include/Solvers.h"
#include "Theme.h"

namespace {

std::string fixed(double v, int digits) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(digits) << v;
    return out.str();
}

std::string cellName(const Position& p) {
    return "(" + std::to_string(p.row) + "," + std::to_string(p.col) + ")";
}

sf::Color terrainColor(int weight) {
    return mix(theme::TerrainLight, theme::TerrainDark, static_cast<float>(weight - 2) / 7.f);
}

}  // namespace

// ---------------------------------------------------------------------------
//  Geometry
// ---------------------------------------------------------------------------
sf::FloatRect App::canvasRect() const {
    const float x = theme::Sidebar + theme::Pad;
    const float y = theme::TopBar + theme::Pad;
    return sf::FloatRect({x, y}, {winSize_.x - theme::Sidebar - theme::RightPane - 2 * theme::Pad,
                                  winSize_.y - theme::TopBar - theme::BottomBar - 2 * theme::Pad});
}

float App::cellSize() const {
    const sf::FloatRect c = canvasRect();
    float cs = std::floor(std::min(c.size.x / maze_.getCols(), c.size.y / maze_.getRows()));
    return std::max(cs, 2.f);
}

sf::Vector2f App::gridOrigin() const {
    const sf::FloatRect c = canvasRect();
    const float cs = cellSize();
    return {std::floor(c.position.x + (c.size.x - cs * maze_.getCols()) / 2.f),
            std::floor(c.position.y + (c.size.y - cs * maze_.getRows()) / 2.f)};
}

// ---------------------------------------------------------------------------
//  Frame
// ---------------------------------------------------------------------------
void App::draw() {
    window_.clear(theme::Background);
    drawGrid();
    drawTopBar();
    drawSidebar();
    drawRightPanel();
    drawBottomBar();
    for (const Button& b : buttons_) drawButton(window_, font_, b, b.rect.contains(mouse_));
    if (showComparison_) drawComparison();
    if (showHelp_) drawHelp();
    window_.display();
}

void App::drawTopBar() {
    drawRect(window_, sf::FloatRect({0.f, 0.f}, {winSize_.x, theme::TopBar}), theme::Panel);
    drawRect(window_, sf::FloatRect({0.f, theme::TopBar - 1.f}, {winSize_.x, 1.f}), theme::PanelEdge);
    drawText(window_, font_, "MAZE-X", {theme::Pad + 2.f, 8.f}, 24, theme::Accent);
    drawText(window_, font_, "Pathfinding & DSA visualizer", {theme::Pad + 2.f, 36.f}, 12, theme::Muted);

    // Status line: hover tip wins, otherwise the latest status message.
    const bool tip = !hoverTip_.empty();
    drawText(window_, font_, tip ? hoverTip_ : status_, {theme::Sidebar + theme::Pad, 19.f}, 15,
             tip ? theme::Muted : statusColor_);

    // State pill on the right.
    const std::string state = stateText();
    sf::Color c = theme::Muted;
    if (state == "Searching") c = theme::Accent;
    else if (state == "Solved") c = theme::Good;
    else if (state == "No path") c = theme::Bad;
    else if (state == "Paused") c = theme::Warn;
    const float pw = 120.f;
    sf::FloatRect pill({winSize_.x - pw - theme::Pad, 14.f}, {pw, 30.f});
    drawRect(window_, pill, theme::Background, c, 1.5f);
    drawText(window_, font_, state, {pill.position.x, 19.f}, 15, c, Align::Center, pw);
}

void App::drawSidebar() {
    drawRect(window_, sf::FloatRect({0.f, theme::TopBar}, {theme::Sidebar, winSize_.y - theme::TopBar}), theme::Panel);
    drawRect(window_, sf::FloatRect({theme::Sidebar - 1.f, theme::TopBar}, {1.f, winSize_.y}), theme::PanelEdge);
    for (const auto& [text, pos] : sectionLabels_)
        drawText(window_, font_, text, pos, 11, theme::Muted);

    drawText(window_, font_, sizeText(), {sizeLabelRect_.position.x, sizeLabelRect_.position.y + 6.f}, 14,
             theme::Text, Align::Center, sizeLabelRect_.size.x);

    // Brush: colour swatch + cost.
    const sf::FloatRect br = brushLabelRect_;
    const std::string label = "cost " + std::to_string(brushWeight_);
    const float tw = textWidth(font_, label, 14);
    const float sx = br.position.x + (br.size.x - tw - 22.f) / 2.f;
    drawRect(window_, sf::FloatRect({sx, br.position.y + 8.f}, {15.f, 15.f}), terrainColor(brushWeight_),
             theme::PanelEdge, 1.f);
    drawText(window_, font_, label, {sx + 22.f, br.position.y + 6.f}, 14, theme::Text);
}

// ---------------------------------------------------------------------------
//  The grid (one vertex array = one draw call, fast even for 61 x 101)
// ---------------------------------------------------------------------------
void App::drawGrid() {
    const sf::FloatRect canvas = canvasRect();
    drawRect(window_, canvas, sf::Color(19, 23, 31));

    const float cs = cellSize();
    const sf::Vector2f o = gridOrigin();
    const float gap = cs >= 7.f ? 1.f : 0.f;
    const bool showPath = playback_.loaded() && playback_.finished();

    sf::VertexArray cells(sf::PrimitiveType::Triangles);
    for (int r = 0; r < maze_.getRows(); ++r) {
        for (int c = 0; c < maze_.getCols(); ++c) {
            Position p(r, c);
            const int idx = maze_.toIndex(p);
            const int w = maze_.getWeight(p);

            sf::Color color = maze_.isWall(p) ? theme::Wall : (w > 1 ? terrainColor(w) : theme::Empty);
            switch (playback_.visualAt(idx)) {
                case CellVisual::Frontier: color = mix(color, theme::Frontier, 0.85f); break;
                case CellVisual::Visited:  color = mix(color, theme::Visited, w > 1 ? 0.55f : 0.8f); break;
                case CellVisual::Current:  color = theme::Current; break;
                case CellVisual::Path:     color = mix(color, theme::Path, w > 1 ? 0.8f : 1.f); break;
                case CellVisual::None:     break;
            }
            if (p == maze_.getStart()) color = theme::Start;
            if (p == maze_.getEnd())   color = theme::End;

            appendQuad(cells, sf::FloatRect({o.x + c * cs, o.y + r * cs}, {cs - gap, cs - gap}), color);
        }
    }
    window_.draw(cells);

    // Route line through the centres of the final path.
    if (showPath && playback_.result().found) {
        const auto& path = playback_.result().path;
        const float t = std::max(2.f, cs * 0.22f);
        sf::VertexArray line(sf::PrimitiveType::Triangles);
        for (std::size_t i = 1; i < path.size(); ++i) {
            sf::Vector2f a{o.x + path[i - 1].col * cs + cs / 2.f, o.y + path[i - 1].row * cs + cs / 2.f};
            sf::Vector2f b{o.x + path[i].col * cs + cs / 2.f, o.y + path[i].row * cs + cs / 2.f};
            sf::Vector2f lo{std::min(a.x, b.x) - t / 2.f, std::min(a.y, b.y) - t / 2.f};
            sf::Vector2f hi{std::max(a.x, b.x) + t / 2.f, std::max(a.y, b.y) + t / 2.f};
            appendQuad(line, sf::FloatRect(lo, hi - lo), sf::Color(161, 98, 7));
        }
        window_.draw(line);
    }

    // Terrain costs and S/E letters when the cells are big enough to read.
    if (cs >= 18.f) {
        const unsigned digit = static_cast<unsigned>(cs * 0.48f);
        for (int r = 0; r < maze_.getRows(); ++r)
            for (int c = 0; c < maze_.getCols(); ++c) {
                Position p(r, c);
                int w = maze_.getWeight(p);
                if (w > 1 && !maze_.isWall(p) && p != maze_.getStart() && p != maze_.getEnd())
                    drawText(window_, mono_, std::to_string(w), {o.x + c * cs, o.y + r * cs + cs * 0.18f}, digit,
                             sf::Color(70, 45, 15), Align::Center, cs - gap);
            }
    }
    if (cs >= 12.f) {
        const unsigned letter = static_cast<unsigned>(cs * 0.55f);
        for (auto [p, ch] : {std::pair{maze_.getStart(), "S"}, std::pair{maze_.getEnd(), "E"}})
            drawText(window_, font_, ch, {o.x + p.col * cs, o.y + p.row * cs + cs * 0.14f}, letter,
                     sf::Color::White, Align::Center, cs - gap);
    }

    // Hover outline.
    if (hover_ && !showComparison_ && !showHelp_) {
        sf::RectangleShape h({cs - gap, cs - gap});
        h.setPosition({o.x + hover_->col * cs, o.y + hover_->row * cs});
        h.setFillColor(sf::Color::Transparent);
        h.setOutlineColor(theme::Accent);
        h.setOutlineThickness(2.f);
        window_.draw(h);
    }
}

// ---------------------------------------------------------------------------
//  Right panel: algorithm info, live metrics, live data structure, legend
// ---------------------------------------------------------------------------
void App::drawRightPanel() {
    const float x0 = winSize_.x - theme::RightPane;
    const float w = theme::RightPane - 2 * theme::Pad;
    const float x = x0 + theme::Pad;
    drawRect(window_, sf::FloatRect({x0, theme::TopBar}, {theme::RightPane, winSize_.y - theme::TopBar}), theme::Panel);
    drawRect(window_, sf::FloatRect({x0, theme::TopBar}, {1.f, winSize_.y}), theme::PanelEdge);

    const AlgorithmInfo& algo = allAlgorithms()[algorithm_];
    float y = theme::TopBar + 12.f;
    drawText(window_, font_, algo.name, {x, y}, 24, theme::Text);
    drawText(window_, mono_, algo.complexity, {x, y + 7.f}, 13, theme::Muted, Align::Right, w);
    y += 34.f;
    drawText(window_, font_, algo.dataStructure, {x, y}, 15, theme::Accent);
    y += 24.f;
    y += drawWrapped(window_, font_, algo.guarantee, {x, y}, 13, theme::Muted, w, 3.f) + 8.f;
    drawRect(window_, sf::FloatRect({x, y}, {w, 1.f}), theme::PanelEdge);
    y += 10.f;

    // Two-column "label  value" rows.
    auto row = [&](const std::string& label, const std::string& value, sf::Color vc = theme::Text) {
        drawText(window_, font_, label, {x, y}, 14, theme::Muted);
        drawText(window_, mono_, value, {x, y}, 14, vc, Align::Right, w);
        y += 21.f;
    };

    if (!playback_.loaded()) {
        drawText(window_, font_, "MAZE", {x, y}, 11, theme::Muted);
        y += 20.f;
        row("Size", std::to_string(maze_.getRows()) + " x " + std::to_string(maze_.getCols()));
        row("Walls", std::to_string(maze_.wallCount()));
        row("Terrain", maze_.isWeighted() ? "weighted (min cost " + std::to_string(maze_.minWeight()) + ")" : "uniform (cost 1)");
        row("Start", cellName(maze_.getStart()), theme::Start);
        row("End", cellName(maze_.getEnd()), theme::End);
        y += 8.f;
        y += drawWrapped(window_, font_,
                         "Press Enter (Solve) to run " + algo.name + ". The search runs once at full speed, "
                         "then its recorded steps are replayed here, so animation speed never changes the result.",
                         {x, y}, 13, theme::Muted, w, 3.f);
    } else {
        const SolverResult& r = playback_.result();
        drawText(window_, font_, "LIVE", {x, y}, 11, theme::Muted);
        y += 20.f;
        // Progress bar
        const float prog = playback_.eventCount() ? static_cast<float>(playback_.eventIndex()) / playback_.eventCount() : 0.f;
        drawRect(window_, sf::FloatRect({x, y}, {w, 6.f}), theme::Button);
        drawRect(window_, sf::FloatRect({x, y}, {w * prog, 6.f}), theme::Accent);
        y += 12.f;
        row("Step", std::to_string(playback_.eventIndex()) + " / " + std::to_string(playback_.eventCount()));
        row("Explored", std::to_string(playback_.exploredSoFar()) + " / " + std::to_string(r.nodesVisited));
        row("Frontier size", std::to_string(playback_.frontierSize()), theme::Frontier);
        row("Current", playback_.hasCurrent() ? cellName(playback_.current()) : "-", theme::Current);
        drawText(window_, mono_, playback_.lastAction(), {x, y}, 13, theme::Text);
        y += 24.f;

        if (playback_.finished()) {
            drawRect(window_, sf::FloatRect({x, y}, {w, 1.f}), theme::PanelEdge);
            y += 10.f;
            drawText(window_, font_, "RESULT", {x, y}, 11, theme::Muted);
            y += 20.f;
            row("Path found", r.found ? "yes" : "no", r.found ? theme::Good : theme::Bad);
            if (r.found) {
                row("Path length", std::to_string(r.pathLength) + " moves");
                row("Path cost", std::to_string(static_cast<int>(r.pathCost)));
            }
            row("Nodes explored", std::to_string(r.nodesVisited));
            row("Max frontier", std::to_string(r.maxFrontier));
            row("Solver time", fixed(r.elapsedMs, 3) + " ms");
            drawText(window_, font_, "median of 5 runs, solver only (no drawing)", {x, y}, 11, theme::Muted);
            y += 18.f;
        }
    }

    // ---- Live data structure ----------------------------------------------
    const float legendTop = winSize_.y - 118.f;
    if (playback_.loaded() && y < legendTop - 60.f) {
        y += 6.f;
        drawRect(window_, sf::FloatRect({x, y}, {w, 1.f}), theme::PanelEdge);
        y += 10.f;
        std::string title = "QUEUE  -  front first (next to pop)";
        if (playback_.kind() == ContainerKind::Stack) title = "STACK  -  top first (next to pop)";
        if (playback_.kind() == ContainerKind::PriorityQueue) title = "PRIORITY QUEUE  -  smallest first";
        drawText(window_, font_, title, {x, y}, 11, theme::Muted);
        y += 20.f;

        const auto entries = playback_.displayOrder();
        const float rowH = 19.f;
        const int fit = std::max(0, static_cast<int>((legendTop - 24.f - y) / rowH));
        const int shown = std::min(fit, static_cast<int>(entries.size()));
        const std::string pname = (algo.name == "A*") ? "f=" : "g=";
        for (int i = 0; i < shown; ++i) {
            const FrontierEntry& e = entries[i];
            if (i == 0) drawRect(window_, sf::FloatRect({x, y - 1.f}, {w, rowH}), sf::Color(14, 116, 144, 90));
            drawText(window_, mono_, std::to_string(i + 1), {x + 4.f, y}, 13, theme::Muted);
            drawText(window_, mono_, cellName(e.cell), {x + 40.f, y}, 13, theme::Text);
            if (playback_.kind() == ContainerKind::PriorityQueue)
                drawText(window_, mono_, pname + fixed(e.priority, 0), {x, y}, 13, theme::Accent, Align::Right, w - 6.f);
            y += rowH;
        }
        if (entries.empty())
            drawText(window_, font_, "(empty)", {x, y}, 13, theme::Muted);
        else if (static_cast<int>(entries.size()) > shown)
            drawText(window_, font_, "+ " + std::to_string(entries.size() - shown) + " more", {x, y}, 13, theme::Muted);
    }

    // ---- Legend --------------------------------------------------------------
    float ly = legendTop;
    drawRect(window_, sf::FloatRect({x, ly}, {w, 1.f}), theme::PanelEdge);
    ly += 10.f;
    const std::vector<std::pair<sf::Color, std::string>> legend = {
        {theme::Start, "Start"}, {theme::End, "End"}, {theme::Wall, "Wall"},
        {theme::Empty, "Walkable"}, {terrainColor(6), "Terrain (cost)"}, {theme::Frontier, "Frontier"},
        {theme::Visited, "Explored"}, {theme::Current, "Current"}, {theme::Path, "Final path"}};
    const float colW = w / 3.f;
    for (std::size_t i = 0; i < legend.size(); ++i) {
        float lx = x + colW * static_cast<float>(i % 3);
        float yy = ly + 24.f * static_cast<float>(i / 3);
        drawRect(window_, sf::FloatRect({lx, yy + 2.f}, {14.f, 14.f}), legend[i].first, theme::PanelEdge, 1.f);
        drawText(window_, font_, legend[i].second, {lx + 20.f, yy}, 12, theme::Text);
    }
}

void App::drawBottomBar() {
    const float x0 = theme::Sidebar;
    const float w = winSize_.x - theme::Sidebar - theme::RightPane;
    const float y0 = winSize_.y - theme::BottomBar;
    drawRect(window_, sf::FloatRect({x0, y0}, {w, theme::BottomBar}), theme::Panel);
    drawRect(window_, sf::FloatRect({x0, y0}, {w, 1.f}), theme::PanelEdge);
    drawText(window_, font_, speedText(), {speedLabelRect_.position.x, speedLabelRect_.position.y + 9.f}, 14,
             theme::Text, Align::Center, speedLabelRect_.size.x);

    // Progress bar in whatever space is left on the right.
    const float px = speedLabelRect_.position.x + speedLabelRect_.size.x + 34.f + 16.f;
    const float pw = x0 + w - theme::Pad - px;
    if (pw > 70.f && playback_.loaded()) {
        const float prog = playback_.eventCount() ? static_cast<float>(playback_.eventIndex()) / playback_.eventCount() : 0.f;
        const float py = y0 + theme::BottomBar / 2.f - 3.f;
        drawRect(window_, sf::FloatRect({px, py}, {pw, 6.f}), theme::Button);
        drawRect(window_, sf::FloatRect({px, py}, {pw * prog, 6.f}), theme::Accent);
    }
}

// ---------------------------------------------------------------------------
//  Overlays
// ---------------------------------------------------------------------------
void App::drawComparison() {
    drawRect(window_, sf::FloatRect({0.f, 0.f}, winSize_), sf::Color(0, 0, 0, 170));
    const float pw = 820.f, ph = 470.f;
    const sf::Vector2f p0{(winSize_.x - pw) / 2.f, (winSize_.y - ph) / 2.f};
    drawRect(window_, sf::FloatRect(p0, {pw, ph}), theme::Panel, theme::Accent, 1.5f);

    float y = p0.y + 18.f;
    const float x = p0.x + 24.f, w = pw - 48.f;
    drawText(window_, font_, "Comparison on the same maze", {x, y}, 22, theme::Text);
    drawText(window_, font_, std::to_string(maze_.getRows()) + " x " + std::to_string(maze_.getCols()) +
             (maze_.isWeighted() ? ", weighted terrain" : ", unweighted"), {x, y + 6.f}, 14, theme::Muted, Align::Right, w);
    y += 46.f;

    const std::vector<std::string> head = {"Algorithm", "Found", "Explored", "Moves", "Cost", "Max frontier", "Time (ms)"};
    const std::vector<float> colX = {0.f, 150.f, 240.f, 350.f, 440.f, 530.f, 660.f};
    for (std::size_t i = 0; i < head.size(); ++i)
        drawText(window_, font_, head[i], {x + colX[i], y}, 13, theme::Muted);
    y += 24.f;
    drawRect(window_, sf::FloatRect({x, y}, {w, 1.f}), theme::PanelEdge);
    y += 8.f;

    int maxNodes = 1;
    double bestCost = 1e18;
    for (const auto& r : comparison_) {
        maxNodes = std::max(maxNodes, r.nodesVisited);
        if (r.found) bestCost = std::min(bestCost, r.pathCost);
    }
    for (const auto& r : comparison_) {
        const bool optimal = r.found && r.pathCost == bestCost;
        std::vector<std::string> cells = {
            r.algorithm, r.found ? "yes" : "no", std::to_string(r.nodesVisited),
            r.found ? std::to_string(r.pathLength) : "-", r.found ? std::to_string(static_cast<int>(r.pathCost)) : "-",
            std::to_string(r.maxFrontier), fixed(r.elapsedMs, 3)};
        for (std::size_t i = 0; i < cells.size(); ++i) {
            sf::Color c = theme::Text;
            if (i == 4 && optimal) c = theme::Good;
            if (i == 1 && !r.found) c = theme::Bad;
            drawText(window_, i == 0 ? font_ : mono_, cells[i], {x + colX[i], y}, 15, c);
        }
        y += 28.f;
    }

    y += 10.f;
    drawText(window_, font_, "Cells explored (fewer = less work)", {x, y}, 13, theme::Muted);
    y += 24.f;
    for (const auto& r : comparison_) {
        drawText(window_, font_, r.algorithm, {x, y}, 13, theme::Text);
        const float bw = (w - 160.f) * static_cast<float>(r.nodesVisited) / static_cast<float>(maxNodes);
        drawRect(window_, sf::FloatRect({x + 90.f, y + 3.f}, {std::max(bw, 2.f), 14.f}), theme::Visited);
        drawText(window_, mono_, std::to_string(r.nodesVisited), {x + 98.f + bw, y}, 13, theme::Muted);
        y += 24.f;
    }
    y += 8.f;
    drawWrapped(window_, font_,
                "Every algorithm ran on an identical copy of this maze with the same neighbour order "
                "(Up, Right, Down, Left). Green cost = optimal. Time is the median of 7 runs on this PC and is "
                "noisy on small mazes - compare explored cells and cost first.   Click or Esc to close.",
                {x, y}, 12, theme::Muted, w, 3.f);
}

void App::drawHelp() {
    drawRect(window_, sf::FloatRect({0.f, 0.f}, winSize_), sf::Color(0, 0, 0, 170));
    const float pw = 640.f, ph = 520.f;
    const sf::Vector2f p0{(winSize_.x - pw) / 2.f, (winSize_.y - ph) / 2.f};
    drawRect(window_, sf::FloatRect(p0, {pw, ph}), theme::Panel, theme::Accent, 1.5f);
    float y = p0.y + 18.f;
    const float x = p0.x + 28.f;
    drawText(window_, font_, "Keyboard and mouse", {x, y}, 22, theme::Text);
    y += 44.f;
    const std::vector<std::pair<std::string, std::string>> keys = {
        {"1 2 3 4", "Select DFS / BFS / Dijkstra / A*"},
        {"Enter", "Solve with the selected algorithm"},
        {"Space", "Play / pause"},
        {"-> or N", "Step one event"},
        {"F", "Finish (jump to the end)"},
        {"R / Esc", "Reset search (keeps the maze)"},
        {"Up / Down", "Faster / slower animation"},
        {"G  O  T", "Generate maze / random obstacles / random terrain"},
        {"C", "Compare all four algorithms"},
        {"W E S D B", "Tool: wall / erase / start / end / terrain brush"},
        {"[  ]", "Terrain brush cost - / +"},
        {"Left drag", "Use the current tool"},
        {"Right drag", "Erase"},
        {"Ctrl+S / Ctrl+L", "Save / load saves/maze.txt"},
        {"H", "This help"},
    };
    for (const auto& [k, d] : keys) {
        drawText(window_, mono_, k, {x, y}, 14, theme::Accent);
        drawText(window_, font_, d, {x + 170.f, y}, 14, theme::Text);
        y += 26.f;
    }
    drawText(window_, font_, "Click or Esc to close", {x, p0.y + ph - 34.f}, 12, theme::Muted);
}
