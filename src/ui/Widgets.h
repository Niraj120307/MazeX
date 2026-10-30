// ============================================================================
//  Widgets.h  -  Minimal immediate-style UI helpers on top of SFML 3
//  (SFML has no buttons or text wrapping, so we build the few we need.)
// ============================================================================

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct Button {
    sf::FloatRect rect;
    std::string label;
    std::function<void()> onClick;
    std::function<bool()> isActive;    // highlighted (e.g. selected algorithm / tool)
    std::function<bool()> isEnabled;   // greyed out when false
    std::string tooltip;               // shown in the status line on hover
    std::function<std::string()> dynamicLabel;   // optional: label that changes (e.g. Play/Pause)

    std::string text() const { return dynamicLabel ? dynamicLabel() : label; }

    bool active()  const { return isActive && isActive(); }
    bool enabled() const { return !isEnabled || isEnabled(); }
};

enum class Align { Left, Center, Right };

// Text drawing helpers. `font` must outlive the call (sf::Text keeps a reference).
float drawText(sf::RenderTarget& target, const sf::Font& font, const std::string& text,
               sf::Vector2f pos, unsigned size, sf::Color color, Align align = Align::Left,
               float boxWidth = 0.f);
float textWidth(const sf::Font& font, const std::string& text, unsigned size);

// Word-wraps `text` into lines no wider than maxWidth. Returns the height used.
float drawWrapped(sf::RenderTarget& target, const sf::Font& font, const std::string& text,
                  sf::Vector2f pos, unsigned size, sf::Color color, float maxWidth, float lineGap = 4.f);

void drawRect(sf::RenderTarget& target, sf::FloatRect rect, sf::Color fill,
              sf::Color outline = sf::Color::Transparent, float outlineThickness = 0.f);

void drawButton(sf::RenderTarget& target, const sf::Font& font, const Button& b, bool hovered);

// Adds an axis-aligned quad (two triangles) to a vertex array - used for the grid.
void appendQuad(sf::VertexArray& va, sf::FloatRect rect, sf::Color color);

sf::Color mix(sf::Color a, sf::Color b, float t);   // t = 0 -> a, t = 1 -> b
