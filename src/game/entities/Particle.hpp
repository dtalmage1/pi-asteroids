#pragma once
#include "game/Vec2.hpp"
#include <cstddef>

namespace ast {

constexpr std::size_t kMaxParticles = 128;

struct Particle {
    Vec2  position;
    Vec2  velocity;
    float lifetime    = 0.0F;
    float maxLifetime = 0.0F;
    bool  active      = false;
};

} // namespace ast
