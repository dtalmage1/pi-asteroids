#pragma once
#include <vector>
#include "game/Colour.hpp"
#include "game/Vec2.hpp"

namespace ast {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    IRenderer()                              = default;
    IRenderer(const IRenderer&)             = delete;
    IRenderer& operator=(const IRenderer&)  = delete;
    IRenderer(IRenderer&&)                  = delete;
    IRenderer& operator=(IRenderer&&)       = delete;

    virtual void clear(Colour background) = 0;
    virtual void drawLine(Vec2 a, Vec2 b, Colour c) = 0;
    virtual void drawLineStrip(const std::vector<Vec2>& points, Colour c, bool closed) = 0;
    virtual void present() = 0;
    virtual Vec2 screenSize() const = 0;
};

} // namespace ast
