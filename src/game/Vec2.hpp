#pragma once
#include <cmath>

namespace ast {

struct Vec2 {
    float x = 0.F;
    float y = 0.F;

    Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }

    float length() const { return std::sqrt(x * x + y * y); }

    Vec2 normalised() const {
        const float len = length();
        if (len == 0.F) {
            return {0.F, 0.F};
        }
        return {x / len, y / len};
    }

    static float distance(Vec2 a, Vec2 b) {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

} // namespace ast
