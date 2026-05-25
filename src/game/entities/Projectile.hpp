#pragma once
#include "game/Vec2.hpp"
#include <cstddef>

namespace ast {

enum class ProjectileOwner { Player, Saucer };

constexpr std::size_t kMaxProjectiles = 4;

struct Projectile {
    Vec2            position;
    Vec2            velocity;
    float           lifetime = 0.0F; // counts down; deactivates at ≤ 0
    ProjectileOwner owner    = ProjectileOwner::Player;
    bool            active   = false;
};

} // namespace ast
