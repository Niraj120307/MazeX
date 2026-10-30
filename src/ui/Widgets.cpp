// ============================================================================
//  Widgets.cpp
// ============================================================================

#include "Widgets.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include "Theme.h"

float textWidth(const sf::Font& font, const std::string& text, unsigned size) {
    sf::Text t(font, text, size);
    return t.getLocalBounds().size.x;
}

float drawText(sf::RenderTarget& target, const sf::Font& font, const std::string& text,
               sf::Vector2f pos, unsigned size, sf::Color color, Align align, float boxWidth) {
    sf::Text t(font, text, size);
    t.setFillColor(color);
    float w = t.getLocalBounds().size.x;
    float x = pos.x;
    if (align == Align::Center) x = pos.x + (boxWidth - w) / 2.f;
    else if (align == Align::Right) x = pos.x + boxWidth - w;
    t.setPosition({std::round(x), std::round(pos.y)});
    target.draw(t);
    return w;
}

float drawWrapped(sf::RenderTarget& target, const sf::Font& font, const std::string& text,
                  sf::Vector2f pos, unsigned size, sf::Color color, float maxWidth, float lineGap) {
    std::istringstream words(text);
    std::string word, line;
    float y = pos.y;
    const float lineHeight = static_cast<float>(size) + lineGap;
    while (words >> word) {
        std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && textWidth(font, candidate, size) > maxWidth) {
            drawText(target, font, line, {pos.x, y}, size, color);
            y += lineHeight;
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) {
        drawText(target, font, line, {pos.x, y}, size, color);
        y += lineHeight;
    }
    return y - pos.y;
}

void drawRect(sf::RenderTarget& target, sf::FloatRect rect, sf::Color fill,
              sf::Color outline, float outlineThickness) {
    sf::RectangleShape r(rect.size);
    r.setPosition(rect.position);
    r.setFillColor(fill);
    if (outlineThickness > 0.f) {
        r.setOutlineColor(outline);
        r.setOutlineThickness(outlineThickness);
    }
    target.draw(r);
}

void drawButton(sf::RenderTarget& target, const sf::Font& font, const Button& b, bool hovered) {
    sf::Color fill = theme::Button;
    sf::Color text = theme::Text;
    if (!b.enabled())      { fill = theme::ButtonOff; text = theme::Muted; }
    else if (b.active())   { fill = theme::ButtonActive; }
    else if (hovered)      { fill = theme::ButtonHover; }
    drawRect(target, b.rect, fill, hovered && b.enabled() ? theme::Accent : theme::PanelEdge, 1.f);
    const unsigned size = 14;
    float y = b.rect.position.y + (b.rect.size.y - static_cast<float>(size)) / 2.f - 3.f;
    drawText(target, font, b.text(), {b.rect.position.x, y}, size, text, Align::Center, b.rect.size.x);
}

void appendQuad(sf::VertexArray& va, sf::FloatRect r, sf::Color color) {
    const sf::Vector2f a = r.position;
    const sf::Vector2f b = {r.position.x + r.size.x, r.position.y};
    const sf::Vector2f c = r.position + r.size;
    const sf::Vector2f d = {r.position.x, r.position.y + r.size.y};
    va.append(sf::Vertex{a, color});
    va.append(sf::Vertex{b, color});
    va.append(sf::Vertex{c, color});
    va.append(sf::Vertex{a, color});
    va.append(sf::Vertex{c, color});
    va.append(sf::Vertex{d, color});
}

sf::Color mix(sf::Color a, sf::Color b, float t) {
    t = std::clamp(t, 0.f, 1.f);
    auto lerp = [t](std::uint8_t x, std::uint8_t y) {
        return static_cast<std::uint8_t>(std::round(x + (y - x) * t));
    };
    return sf::Color(lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a));
}
