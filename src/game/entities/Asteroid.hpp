#pragma once
#include "game/Vec2.hpp"
#include <cstdint>
#include <vector>

namespace ast {

enum class AsteroidSize { Large, Medium, Small };

struct Asteroid {
    Vec2              position;
    Vec2              velocity;
    float             angle      = 0.0F;
    float             angularVel = 0.0F;
    AsteroidSize      size       = AsteroidSize::Large;
    std::vector<Vec2> shape;
    bool              active     = false;

    static constexpr float kRadiusLarge  = 48.0F;
    static constexpr float kRadiusMedium = 24.0F;
    static constexpr float kRadiusSmall  = 12.0F;

    static float radius(AsteroidSize s) noexcept;
};

// Generate a randomised local-space polygon for an asteroid.
// vertices: number of vertices (8–12 typical).
// seed: controls jitter; same seed always produces the same shape.
std::vector<Vec2> generateAsteroidShape(AsteroidSize size, int vertices, uint32_t seed);

} // namespace ast
