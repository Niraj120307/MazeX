// ============================================================================
//  Controls.cpp  -  Button layout and actions (rebuilt whenever the window resizes)
// ============================================================================

#include "App.h"
#include <algorithm>
#include "../../include/Solvers.h"
#include "Theme.h"

void App::buildButtons() {
    buttons_.clear();
    sectionLabels_.clear();

    const float x0 = theme::Pad;
    const float w  = theme::Sidebar - 2 * theme::Pad;
    // 14 rows of buttons + 4 headings must fit the window height: shrink rows on short screens.
    const float rowSpace = winSize_.y - theme::TopBar - 10.f - 4 * 22.f - 3 * 6.f - 10.f;
    const float h  = std::clamp(rowSpace / 14.f - 5.f, 24.f, 31.f);
    const float gap = 5.f;
    const float half = (w - gap) / 2.f;
    float y = theme::TopBar + 10.f;

    auto heading = [&](const std::string& text) {
        sectionLabels_.push_back({text, {x0, y}});
        y += 22.f;
    };
    auto full = [&](const std::string& label, std::function<void()> fn, const std::string& tip,
                    std::function<bool()> active = nullptr, std::function<bool()> enabled = nullptr) {
        buttons_.push_back({sf::FloatRect({x0, y}, {w, h}), label, std::move(fn), std::move(active),
                            std::move(enabled), tip, nullptr});
        y += h + gap;
    };
    auto pair = [&](const std::string& l1, std::function<void()> f1, std::function<bool()> a1, const std::string& t1,
                    const std::string& l2, std::function<void()> f2, std::function<bool()> a2, const std::string& t2) {
        buttons_.push_back({sf::FloatRect({x0, y}, {half, h}), l1, std::move(f1), std::move(a1), nullptr, t1, nullptr});
        buttons_.push_back({sf::FloatRect({x0 + half + gap, y}, {half, h}), l2, std::move(f2), std::move(a2), nullptr, t2, nullptr});
        y += h + gap;
    };
    // [-]  label  [+]
    auto stepper = [&](std::function<void()> minus, std::function<void()> plus, sf::FloatRect& labelRect,
                       const std::string& tip) {
        const float bw = 34.f;
        buttons_.push_back({sf::FloatRect({x0, y}, {bw, h}), "-", std::move(minus), nullptr, nullptr, tip, nullptr});
        buttons_.push_back({sf::FloatRect({x0 + w - bw, y}, {bw, h}), "+", std::move(plus), nullptr, nullptr, tip, nullptr});
        labelRect = sf::FloatRect({x0 + bw, y}, {w - 2 * bw, h});
        y += h + gap;
    };

    // ---- Algorithm ---------------------------------------------------------
    heading("ALGORITHM");
    auto algoActive = [this](int i) { return [this, i] { return algorithm_ == i; }; };
    pair("1  DFS", [this] { selectAlgorithm(0); }, algoActive(0), "Depth-first search - uses a stack",
         "2  BFS", [this] { selectAlgorithm(1); }, algoActive(1), "Breadth-first search - uses a queue");
    pair("3  Dijkstra", [this] { selectAlgorithm(2); }, algoActive(2), "Dijkstra - min-priority queue by cost so far",
         "4  A*", [this] { selectAlgorithm(3); }, algoActive(3), "A* - min-priority queue by f = g + h");
    full("Solve  (Enter)", [this] { solve(); }, "Run the selected algorithm and animate it");
    full("Compare all  (C)", [this] { runComparison(); }, "Run all 4 on the same maze and show a results table");
    y += 6.f;

    // ---- Maze --------------------------------------------------------------
    heading("MAZE");
    full("Generate maze  (G)", [this] { generateMaze(); }, "Recursive backtracking at the size below");
    pair("Obstacles (O)", [this] { generateObstacles(); }, nullptr, "Random walls (always solvable)",
         "Terrain (T)", [this] { addTerrain(); }, nullptr, "Random weighted patches, cost 3-9");
    pair("Clear walls", [this] { clearWalls(); }, nullptr, "Remove all walls and terrain",
         "Reset  (R)", [this] { resetSearch(); }, nullptr, "Clear the search overlay, keep the maze");
    stepper([this] { changeSize(-1); }, [this] { changeSize(+1); }, sizeLabelRect_, "Grid size for Generate");
    y += 6.f;

    // ---- Edit tool ---------------------------------------------------------
    heading("EDIT TOOL  (right-drag = erase)");
    auto toolIs = [this](Tool t) { return [this, t] { return tool_ == t; }; };
    pair("Wall  (W)", [this] { tool_ = Tool::Wall; }, toolIs(Tool::Wall), "Left-drag to draw walls",
         "Erase  (E)", [this] { tool_ = Tool::Erase; }, toolIs(Tool::Erase), "Left-drag to remove walls and terrain");
    pair("Start  (S)", [this] { tool_ = Tool::Start; }, toolIs(Tool::Start), "Click or drag to move the Start",
         "End  (D)", [this] { tool_ = Tool::End; }, toolIs(Tool::End), "Click or drag to move the End");
    full("Terrain brush  (B)", [this] { tool_ = Tool::Terrain; }, "Paint weighted terrain with the cost below",
         toolIs(Tool::Terrain));
    stepper([this] { changeBrush(-1); }, [this] { changeBrush(+1); }, brushLabelRect_, "Terrain brush cost  ( [ and ] )");
    y += 6.f;

    // ---- File --------------------------------------------------------------
    heading("FILE");
    pair("Save  ^S", [this] { saveToFile(); }, nullptr, "Save to saves/maze.txt",
         "Load  ^L", [this] { loadFromFile(); }, nullptr, "Load saves/maze.txt");
    pair("Bench CSV", [this] { exportBenchmark(); }, nullptr, "Benchmark all algorithms on 4 sizes -> benchmarks/results.csv",
         "Help  (H)", [this] { showHelp_ = true; }, nullptr, "Keyboard shortcuts");

    // ---- Bottom playback bar (spans the canvas only) -----------------------
    const float left = theme::Sidebar + theme::Pad;
    // Scale the bottom controls down when the canvas is narrow (natural width ~ 600 px).
    const float canvasW = winSize_.x - theme::Sidebar - theme::RightPane - 2 * theme::Pad;
    const float k = std::clamp(canvasW / 600.f, 0.72f, 1.f);
    const float by = winSize_.y - theme::BottomBar + (theme::BottomBar - 36.f) / 2.f;
    const float bh = 36.f;
    float bx = left;
    auto bottom = [&](float baseWidth, const std::string& label, std::function<void()> fn, const std::string& tip,
                      std::function<std::string()> dyn = nullptr) {
        const float width = baseWidth * k;
        buttons_.push_back({sf::FloatRect({bx, by}, {width, bh}), label, std::move(fn), nullptr, nullptr, tip, std::move(dyn)});
        bx += width + 6.f;
    };
    bottom(108.f, "Play", [this] { togglePlay(); }, "Play / pause the animation (Space)",
           [this] { return playing_ ? std::string("Pause") : std::string("Play"); });
    bottom(84.f, "Step", [this] { stepOnce(); }, "One event forward (-> or N)");
    bottom(92.f, "Restart", [this] { restartPlayback(); }, "Rewind the animation to step 0");
    bottom(84.f, "Finish", [this] { finishPlayback(); }, "Jump to the end (F)");
    bx += 10.f;
    const float labelW = 118.f * k;
    buttons_.push_back({sf::FloatRect({bx, by}, {34.f, bh}), "-", [this] { changeSpeed(-1); }, nullptr, nullptr,
                        "Slower (Down arrow)", nullptr});
    speedLabelRect_ = sf::FloatRect({bx + 34.f, by}, {labelW, bh});
    buttons_.push_back({sf::FloatRect({bx + 34.f + labelW, by}, {34.f, bh}), "+", [this] { changeSpeed(+1); }, nullptr,
                        nullptr, "Faster (Up arrow)", nullptr});
}
