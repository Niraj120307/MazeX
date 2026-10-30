// ============================================================================
//  App.cpp  -  Event/frame loop, editing, solving and file actions
// ============================================================================

#include "App.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include "../../include/Benchmark.h"
#include "../../include/MazeGenerator.h"
#include "../../include/MazeIO.h"
#include "../../include/Solvers.h"
#include "Theme.h"

namespace fs = std::filesystem;

namespace {

// Fit the window to the screen (e.g. 1366 x 768 laptops) instead of a fixed size.
sf::Vector2u initialWindowSize() {
    const sf::Vector2u desk = sf::VideoMode::getDesktopMode().size;
    unsigned w = std::min(1440u, static_cast<unsigned>(desk.x * 0.94f));
    unsigned h = std::min(860u, static_cast<unsigned>(desk.y * 0.88f));
    return {std::max(w, 1024u), std::max(h, 660u)};
}

// Animation speeds in events per second. Speed changes ONLY the replay rate,
// never the solver or its measured time.
const float kSpeeds[] = {2, 5, 10, 20, 40, 80, 160, 320, 640, 1500, 5000};
constexpr int kSpeedCount = static_cast<int>(sizeof(kSpeeds) / sizeof(kSpeeds[0]));

// Grid sizes for "Generate". Odd sizes give the recursive backtracker a full border of rooms.
const sf::Vector2i kSizes[] = {{11, 19}, {21, 35}, {31, 51}, {41, 71}, {61, 101}};
constexpr int kSizeCount = static_cast<int>(sizeof(kSizes) / sizeof(kSizes[0]));

std::string cellName(const Position& p) {
    return "(" + std::to_string(p.row) + "," + std::to_string(p.col) + ")";
}

// Looks for the assets folder next to the exe and in parent folders (build/ layouts).
std::string findAssetDir() {
    for (const char* candidate : {"assets", "../assets", "../../assets", "../../../assets"})
        if (fs::exists(fs::path(candidate) / "fonts")) return candidate;
    return "assets";
}

}  // namespace

App::App()
    : window_(sf::VideoMode(initialWindowSize()), "MAZE-X  |  Maze Generation, Pathfinding & DSA Visualizer"),
      winSize_(sf::Vector2f(initialWindowSize())),
      maze_(kSizes[1].x, kSizes[1].y) {
    window_.setFramerateLimit(60);
    window_.setMinimumSize(sf::Vector2u{1024u, 660u});
    assetDir_ = findAssetDir();
    if (!loadFonts())
        throw std::runtime_error("No font found. Put Inter.ttf in assets/fonts/ (see README).");

    // Start from the prepared demo maze so the demo never depends on random output.
    try {
        maze_ = loadMaze((fs::path(assetDir_) / "mazes" / "demo.txt").string());
        setStatus("Loaded prepared demo maze. Pick an algorithm and press Solve (Enter).", theme::Muted);
    } catch (const std::exception&) {
        generateBacktracker(maze_, seed_, 0.12);
        setStatus("Generated a maze (seed " + std::to_string(seed_) + ").", theme::Muted);
    }
    onResize(window_.getSize());   // lay out for the size the OS actually gave us
}

bool App::loadFonts() {
    const fs::path fonts = fs::path(assetDir_) / "fonts";
    const std::vector<fs::path> uiFonts = {
        fonts / "Inter.ttf", "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/System/Library/Fonts/Supplemental/Arial.ttf"};
    const std::vector<fs::path> monoFonts = {
        fonts / "JetBrainsMono.ttf", "C:/Windows/Fonts/consola.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"};

    bool ui = false, mono = false;
    for (const fs::path& p : uiFonts)   if (!ui && fs::exists(p))   ui = font_.openFromFile(p);
    for (const fs::path& p : monoFonts) if (!mono && fs::exists(p)) mono = mono_.openFromFile(p);
    if (ui && !mono) mono = mono_.openFromFile(uiFonts[0]) || mono_.openFromFile("C:/Windows/Fonts/segoeui.ttf");
    return ui && mono;
}

void App::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        while (const std::optional event = window_.pollEvent())
            handleEvent(*event);
        update(clock.restart().asSeconds());
        draw();
    }
}

// ---------------------------------------------------------------------------
//  Events
// ---------------------------------------------------------------------------
void App::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::Closed>()) { window_.close(); return; }

    if (const auto* r = event.getIf<sf::Event::Resized>()) { onResize(r->size); return; }

    if (const auto* m = event.getIf<sf::Event::MouseMoved>()) {
        mouse_ = sf::Vector2f(m->position);
        hover_ = cellAt(mouse_);
        hoverTip_.clear();
        for (const Button& b : buttons_)
            if (b.rect.contains(mouse_)) hoverTip_ = b.tooltip;
        if (painting_ && hover_ && hover_ != lastPainted_) applyTool(*hover_, erasing_);
        return;
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        const sf::Vector2f p(mb->position);
        if (showHelp_) { showHelp_ = false; return; }
        if (showComparison_) { showComparison_ = false; return; }
        if (mb->button == sf::Mouse::Button::Left) {
            for (Button& b : buttons_) {
                if (b.rect.contains(p)) {
                    if (b.enabled() && b.onClick) b.onClick();
                    return;
                }
            }
        }
        if (auto cell = cellAt(p)) {
            painting_ = true;
            erasing_ = (mb->button == sf::Mouse::Button::Right);
            lastPainted_.reset();
            applyTool(*cell, erasing_);
        }
        return;
    }

    if (event.is<sf::Event::MouseButtonReleased>()) {
        painting_ = false;
        lastPainted_.reset();
        return;
    }

    if (const auto* k = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;
        if (showHelp_ || showComparison_) {
            if (k->code == Key::Escape || k->code == Key::H || k->code == Key::C) { showHelp_ = showComparison_ = false; }
            return;
        }
        switch (k->code) {
            case Key::Num1: selectAlgorithm(0); break;
            case Key::Num2: selectAlgorithm(1); break;
            case Key::Num3: selectAlgorithm(2); break;
            case Key::Num4: selectAlgorithm(3); break;
            case Key::Enter: solve(); break;
            case Key::Space: togglePlay(); break;
            case Key::Right:
            case Key::N:     stepOnce(); break;
            case Key::F:     finishPlayback(); break;
            case Key::R:     resetSearch(); break;
            case Key::G:     generateMaze(); break;
            case Key::O:     generateObstacles(); break;
            case Key::T:     addTerrain(); break;
            case Key::C:     runComparison(); break;
            case Key::S:     if (k->control) saveToFile(); else tool_ = Tool::Start; break;
            case Key::L:     if (k->control) loadFromFile(); break;
            case Key::W:     tool_ = Tool::Wall; break;
            case Key::E:     tool_ = Tool::Erase; break;
            case Key::D:     tool_ = Tool::End; break;
            case Key::B:     tool_ = Tool::Terrain; break;
            case Key::Up:    changeSpeed(+1); break;
            case Key::Down:  changeSpeed(-1); break;
            case Key::LBracket: changeBrush(-1); break;
            case Key::RBracket: changeBrush(+1); break;
            case Key::H:     showHelp_ = true; break;
            case Key::Escape: if (playback_.loaded()) resetSearch(); break;
            default: break;
        }
    }
}

void App::onResize(sf::Vector2u size) {
    winSize_ = sf::Vector2f(size);
    window_.setView(sf::View(sf::FloatRect({0.f, 0.f}, winSize_)));
    buildButtons();
}

// Frame update: play as many events as the speed allows this frame.
// The UI never blocks: no sleep, the window keeps handling input every frame.
void App::update(float dt) {
    if (!playing_ || !playback_.loaded()) return;
    stepBudget_ += dt * kSpeeds[speedIndex_];
    while (stepBudget_ >= 1.f && playback_.step()) stepBudget_ -= 1.f;
    if (playback_.finished()) {
        playing_ = false;
        stepBudget_ = 0.f;
        const SolverResult& r = playback_.result();
        if (r.found)
            setStatus(r.algorithm + ": path found - " + std::to_string(r.pathLength) + " moves, cost " +
                      std::to_string(static_cast<int>(r.pathCost)) + ", " + std::to_string(r.nodesVisited) +
                      " cells explored.", theme::Good);
        else
            setStatus(r.algorithm + ": no path found (" + std::to_string(r.nodesVisited) + " cells explored).",
                      theme::Bad);
    }
}

// ---------------------------------------------------------------------------
//  Search control
// ---------------------------------------------------------------------------
void App::selectAlgorithm(int index) {
    algorithm_ = std::clamp(index, 0, static_cast<int>(allAlgorithms().size()) - 1);
    if (playback_.loaded()) resetSearch();
    setStatus(allAlgorithms()[algorithm_].name + " selected. Press Solve (Enter).", theme::Muted);
}

void App::solve() {
    const AlgorithmInfo& algo = allAlgorithms()[algorithm_];
    // The solver gets a snapshot copy; median of 5 runs so the time shown is stable.
    SolverResult result = runTimed(algo.solve, maze_, 5);
    playback_.load(maze_, result);
    playing_ = true;
    stepBudget_ = 0.f;
    setStatus("Searching with " + algo.name + " ...  Space = pause, -> = step", theme::Accent);
}

void App::togglePlay() {
    if (!playback_.loaded()) { solve(); return; }
    if (playback_.finished()) { restartPlayback(); playing_ = true; return; }
    playing_ = !playing_;
    setStatus(playing_ ? "Playing" : "Paused - press -> or N to step", theme::Muted);
}

void App::stepOnce() {
    if (!playback_.loaded()) { solve(); playing_ = false; }
    playing_ = false;
    playback_.step();
    setStatus("Step " + std::to_string(playback_.eventIndex()) + "/" + std::to_string(playback_.eventCount()) +
              ": " + playback_.lastAction(), theme::Muted);
}

void App::restartPlayback() {
    if (!playback_.loaded()) return;
    playback_.restart();
    playing_ = false;
    setStatus("Rewound to step 0. Space to play.", theme::Muted);
}

void App::finishPlayback() {
    if (!playback_.loaded()) solve();
    playback_.runToEnd();
    playing_ = true;     // let update() post the usual "path found / no path" status
    update(0.f);
}

void App::resetSearch() {
    playback_.clear();
    playing_ = false;
    setStatus("Search cleared (maze kept).", theme::Muted);
}

void App::mazeChanged(const std::string& message) {
    playback_.clear();
    playing_ = false;
    comparison_.clear();
    setStatus(message, theme::Muted);
}

// ---------------------------------------------------------------------------
//  Maze actions
// ---------------------------------------------------------------------------
void App::generateMaze() {
    sf::Vector2i s = kSizes[sizeIndex_];
    maze_ = Maze(s.x, s.y);
    ++seed_;
    generateBacktracker(maze_, seed_, 0.12);
    mazeChanged("Recursive-backtracking maze " + std::to_string(s.x) + "x" + std::to_string(s.y) +
                " (seed " + std::to_string(seed_) + ", 12% extra openings).");
}

void App::generateObstacles() {
    ++seed_;
    generateRandomObstacles(maze_, seed_, 0.30);
    mazeChanged("Random obstacles, 30% density (seed " + std::to_string(seed_) + "). Always solvable.");
}

void App::addTerrain() {
    ++seed_;
    addRandomTerrain(maze_, seed_, 0.30);
    mazeChanged("Random terrain added (cost 3-9). Compare BFS with Dijkstra / A*.");
}

void App::clearWalls() {
    maze_.clearWalls();
    maze_.clearWeights();
    mazeChanged("Walls and terrain cleared.");
}

void App::changeSize(int delta) {
    sizeIndex_ = std::clamp(sizeIndex_ + delta, 0, kSizeCount - 1);
    generateMaze();
}

void App::changeSpeed(int delta) {
    speedIndex_ = std::clamp(speedIndex_ + delta, 0, kSpeedCount - 1);
    setStatus("Animation speed: " + std::to_string(static_cast<int>(kSpeeds[speedIndex_])) +
              " steps/s (does not affect solver timing).", theme::Muted);
}

void App::changeBrush(int delta) {
    brushWeight_ = std::clamp(brushWeight_ + delta, 2, Maze::kMaxWeight);
    tool_ = Tool::Terrain;
    setStatus("Terrain brush cost: " + std::to_string(brushWeight_), theme::Muted);
}

void App::runComparison() {
    comparison_ = compareAll(maze_, 7);   // same snapshot, median of 7 runs each
    showComparison_ = true;
    setStatus("Compared all four algorithms on the same maze.", theme::Accent);
}

void App::saveToFile() {
    fs::create_directories("saves");
    const std::string file = "saves/maze.txt";
    if (saveMaze(maze_, file)) setStatus("Saved to " + file, theme::Good);
    else setStatus("Could not write " + file, theme::Bad);
}

void App::loadFromFile() {
    const std::string file = "saves/maze.txt";
    try {
        maze_ = loadMaze(file);
        mazeChanged("Loaded " + file + " (" + std::to_string(maze_.getRows()) + "x" +
                    std::to_string(maze_.getCols()) + ")");
    } catch (const std::exception& e) {
        setStatus(std::string("Load failed: ") + e.what() + "  (save one first with Ctrl+S)", theme::Bad);
    }
}

void App::exportBenchmark() {
    fs::create_directories("benchmarks");
    const std::string file = "benchmarks/results.csv";
    auto rows = runBenchmarks({21, 41, 61, 81}, 5, 2026);
    if (writeBenchmarkCsv(rows, file))
        setStatus("Benchmark: " + std::to_string(rows.size()) + " rows (4 sizes x 3 maze types x 4 algorithms, "
                  "median of 5) -> " + file, theme::Good);
    else
        setStatus("Could not write " + file, theme::Bad);
}

// ---------------------------------------------------------------------------
//  Mouse editing
// ---------------------------------------------------------------------------
std::optional<Position> App::cellAt(sf::Vector2f pixel) const {
    const float cs = cellSize();
    const sf::Vector2f o = gridOrigin();
    if (pixel.x < o.x || pixel.y < o.y) return std::nullopt;
    int col = static_cast<int>((pixel.x - o.x) / cs);
    int row = static_cast<int>((pixel.y - o.y) / cs);
    Position p(row, col);
    if (!maze_.isInside(p)) return std::nullopt;
    return p;
}

void App::applyTool(Position cell, bool erase) {
    lastPainted_ = cell;
    bool changed = false;
    std::string message;

    if (erase || tool_ == Tool::Erase) {
        changed = maze_.isWall(cell) || maze_.getWeight(cell) > 1;
        maze_.removeWall(cell);
        maze_.setWeight(cell, 1);
        message = "Erased " + cellName(cell);
    } else if (tool_ == Tool::Wall) {
        if (!maze_.isWall(cell)) {
            changed = maze_.setWall(cell);
            if (!changed) { setStatus("Cannot put a wall on Start or End.", theme::Warn); return; }
        }
        message = "Wall at " + cellName(cell);
    } else if (tool_ == Tool::Start) {
        changed = maze_.getStart() != cell && maze_.setStart(cell);
        if (!changed && maze_.getStart() != cell) { setStatus("Start must differ from End.", theme::Warn); return; }
        message = "Start moved to " + cellName(cell);
    } else if (tool_ == Tool::End) {
        changed = maze_.getEnd() != cell && maze_.setEnd(cell);
        if (!changed && maze_.getEnd() != cell) { setStatus("End must differ from Start.", theme::Warn); return; }
        message = "End moved to " + cellName(cell);
    } else if (tool_ == Tool::Terrain) {
        if (maze_.isWall(cell)) { setStatus("Terrain goes on walkable cells (erase the wall first).", theme::Warn); return; }
        changed = maze_.getWeight(cell) != brushWeight_;
        maze_.setWeight(cell, brushWeight_);
        message = "Terrain cost " + std::to_string(brushWeight_) + " at " + cellName(cell);
    }
    if (changed) mazeChanged(message);
}

void App::setStatus(const std::string& text, sf::Color color) {
    status_ = text;
    statusColor_ = color;
}

std::string App::sizeText() const {
    return std::to_string(kSizes[sizeIndex_].x) + " x " + std::to_string(kSizes[sizeIndex_].y);
}

std::string App::speedText() const {
    return std::to_string(static_cast<int>(kSpeeds[speedIndex_])) + " steps/s";
}

std::string App::stateText() const {
    if (!playback_.loaded()) return "Ready";
    if (playback_.finished()) return playback_.result().found ? "Solved" : "No path";
    return playing_ ? "Searching" : "Paused";
}
