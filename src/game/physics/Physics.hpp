#pragma once
#include "game/Vec2.hpp"
#include <cmath>

namespace ast {

// pos + vel * dt
inline Vec2 integratePosition(Vec2 pos, Vec2 vel, float dt) {
    return pos + vel * dt;
}

// Add thrust in the direction the ship is facing.
// angle = 0 points up (screen-negative Y); increases clockwise.
// Nose direction: Vec2(sin(angle), -cos(angle)).
inline Vec2 applyThrust(Vec2 vel, float angle, float accel, float dt) {
    const Vec2 nose{std::sin(angle), -std::cos(angle)};
    return vel + nose * (accel * dt);
}

// Exponential-style drag: reduce speed by drag fraction per second.
// Clamps to zero to avoid sign reversal on large dt.
inline Vec2 applyDrag(Vec2 vel, float drag, float dt) {
    const float factor = 1.0F - drag * dt;
    return vel * (factor < 0.0F ? 0.0F : factor);
}

// Wrap pos into [0, screenSize) on each axis.
inline Vec2 wrapPosition(Vec2 pos, Vec2 screenSize) {
    pos.x = std::fmod(pos.x, screenSize.x);
    if (pos.x < 0.0F) { pos.x += screenSize.x; }
    pos.y = std::fmod(pos.y, screenSize.y);
    if (pos.y < 0.0F) { pos.y += screenSize.y; }
    return pos;
}

// Keep angle in [0, 2π).
inline float wrapAngle(float angle) {
    constexpr float kTwoPi = 6.28318530F;
    angle = std::fmod(angle, kTwoPi);
    if (angle < 0.0F) { angle += kTwoPi; }
    return angle;
}

} // namespace ast
