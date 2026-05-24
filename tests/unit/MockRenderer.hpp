#pragma once
#include <gmock/gmock.h>
#include <vector>
#include "game/IRenderer.hpp"

namespace ast {

class MockRenderer : public IRenderer {
public:
    MOCK_METHOD(void, clear, (Colour background), (override));
    MOCK_METHOD(void, drawLine, (Vec2 a, Vec2 b, Colour c), (override));
    MOCK_METHOD(void, drawLineStrip, (const std::vector<Vec2>& points, Colour c, bool closed), (override));
    MOCK_METHOD(void, present, (), (override));
    MOCK_METHOD(Vec2, screenSize, (), (const, override));
};

} // namespace ast
