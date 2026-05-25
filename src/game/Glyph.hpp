#pragma once
#include "game/Colour.hpp"
#include "game/IRenderer.hpp"
#include "game/Vec2.hpp"
#include <string_view>

namespace ast {

// Renders text using a stroke font built entirely from drawLine calls.
// origin is the top-left corner of the first character.
// charHeight controls the pixel height; width is proportional (0.6× height).
void drawString(IRenderer& renderer, Vec2 origin, float charHeight,
                std::string_view text, Colour colour);

// Returns the total pixel width of text at the given charHeight.
float stringWidth(float charHeight, std::string_view text) noexcept;

} // namespace ast
