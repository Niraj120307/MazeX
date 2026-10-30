// ============================================================================
//  App.h  -  The MAZE-X window: event loop, state, controls and drawing
//
//  The App is the ONLY place where SFML meets the engine:
//    Controls  -> App edits the Maze model
//    Solve     -> App passes a COPY of the maze to a solver, gets a SolverResult
//    Playback  -> replays that result's events for the animation
//    Renderer  -> draws the maze + the playback overlay (never runs algorithm code)
//
//  The class is split over three files to keep each one readable:
//    App.cpp       event/frame loop, editing, solving, file actions
//    Controls.cpp  button layout and what each button does
//    Renderer.cpp  every draw function
// ============================================================================

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../../include/Maze.h"
#include "../../include/Playback.h"
#include "../../include/SolverResult.h"
#include "Widgets.h"

enum class Tool { Wall, Erase, Start, End, Terrain };

class App {
public:
    App();
    void run();

private:
    // ---- App.cpp ------------------------------------------------------------
    bool loadFonts();
    void handleEvent(const sf::Event& event);
    void update(float dt);
    void onResize(sf::Vector2u size);

    void selectAlgorithm(int index);
    void solve();                    // run the selected solver and start playback
    void togglePlay();
    void stepOnce();
    void restartPlayback();
    void finishPlayback();
    void resetSearch();              // clear overlays, keep the maze
    void mazeChanged(const std::string& message);   // any edit: clear search + comparison

    void generateMaze();
    void generateObstacles();
    void addTerrain();
    void clearWalls();
    void changeSize(int delta);
    void changeSpeed(int delta);
    void changeBrush(int delta);
    void runComparison();
    void saveToFile();
    void loadFromFile();
    void exportBenchmark();

    std::optional<Position> cellAt(sf::Vector2f pixel) const;
    void applyTool(Position cell, bool erase);
    void setStatus(const std::string& text, sf::Color color);
    std::string sizeText() const;    // "21 x 35" for the size stepper
    std::string speedText() const;   // "80 steps/s"
    std::string stateText() const;   // Ready / Searching / Paused / Solved / No path

    // ---- Controls.cpp -------------------------------------------------------
    void buildButtons();

    // ---- Renderer.cpp -------------------------------------------------------
    void draw();
    void drawTopBar();
    void drawSidebar();
    void drawGrid();
    void drawRightPanel();
    void drawBottomBar();
    void drawComparison();
    void drawHelp();
    sf::FloatRect canvasRect() const;
    float cellSize() const;
    sf::Vector2f gridOrigin() const;

    // ---- State --------------------------------------------------------------
    sf::RenderWindow window_;
    sf::Font font_;
    sf::Font mono_;
    sf::Vector2f winSize_;

    Maze maze_;
    Playback playback_;
    int algorithm_ = 1;              // index into allAlgorithms() (BFS by default)
    bool playing_ = false;
    int speedIndex_ = 5;             // index into kSpeeds (events per second)
    float stepBudget_ = 0.f;         // fractional events waiting to be played

    Tool tool_ = Tool::Wall;
    int brushWeight_ = 5;
    bool painting_ = false;
    bool erasing_ = false;
    std::optional<Position> hover_;
    std::optional<Position> lastPainted_;

    int sizeIndex_ = 1;
    unsigned seed_ = 2026;

    std::vector<SolverResult> comparison_;
    bool showComparison_ = false;
    bool showHelp_ = false;

    std::string status_ = "Ready";
    sf::Color statusColor_ = sf::Color(138, 147, 166);
    std::string hoverTip_;

    std::vector<Button> buttons_;
    std::vector<std::pair<std::string, sf::Vector2f>> sectionLabels_;   // sidebar headings
    sf::FloatRect sizeLabelRect_, brushLabelRect_, speedLabelRect_;     // text between -/+ buttons
    sf::Vector2f mouse_;
    std::string assetDir_;
};
