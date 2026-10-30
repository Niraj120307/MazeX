// ============================================================================
//  Theme.h  -  Colours and sizes for the SFML interface (one place to tweak)
// ============================================================================

#pragma once

#include <SFML/Graphics/Color.hpp>

namespace theme {

// ---- Window chrome ---------------------------------------------------------
inline const sf::Color Background {15, 18, 24};
inline const sf::Color Panel      {23, 27, 36};
inline const sf::Color PanelEdge  {38, 44, 58};
inline const sf::Color Text       {230, 233, 239};
inline const sf::Color Muted      {138, 147, 166};
inline const sf::Color Accent     {56, 189, 248};    // cyan
inline const sf::Color Good       {74, 222, 128};
inline const sf::Color Bad        {248, 113, 113};
inline const sf::Color Warn       {250, 204, 21};

// ---- Buttons ---------------------------------------------------------------
inline const sf::Color Button       {34, 40, 53};
inline const sf::Color ButtonHover  {46, 54, 71};
inline const sf::Color ButtonActive {14, 116, 144};
inline const sf::Color ButtonOff    {27, 31, 40};

// ---- Maze cells (visual language from the project plan) --------------------
inline const sf::Color Empty    {226, 230, 237};   // neutral walkable tile
inline const sf::Color Wall     {30, 35, 46};      // dark solid tile
inline const sf::Color Frontier {34, 211, 238};    // cyan: waiting in stack/queue/PQ
inline const sf::Color Visited  {147, 180, 230};   // muted blue: explored
inline const sf::Color Current  {236, 72, 153};    // magenta: being expanded right now
inline const sf::Color Path     {250, 204, 21};    // bright yellow: final route
inline const sf::Color Start    {34, 197, 94};     // green marker
inline const sf::Color End      {239, 68, 68};     // red marker
inline const sf::Color TerrainLight {240, 222, 186};   // cost 2
inline const sf::Color TerrainDark  {166, 110, 52};    // cost 9

// ---- Layout (pixels) -------------------------------------------------------
inline constexpr float TopBar    = 58.f;
inline constexpr float Sidebar   = 236.f;
inline constexpr float RightPane = 330.f;
inline constexpr float BottomBar = 64.f;
inline constexpr float Pad       = 12.f;

}  // namespace theme
