#pragma once
#include "game/Vec2.hpp"

namespace ast {

struct Ship {
    Vec2  position;
    Vec2  velocity;
    float angle       = 0.0F;  // radians; 0 = pointing up; increases clockwise
    float invincTimer = 0.0F;  // seconds remaining; > 0 → immune to collision
    bool  thrusting   = false; // drives thrust SFX and flame visual
    bool  active      = true;  // false during explosion/respawn delay

    static constexpr float kRadius = 10.0F; // collision circle radius (px)
};

} // namespace ast
