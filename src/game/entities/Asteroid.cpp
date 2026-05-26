#include "game/entities/Asteroid.hpp"
#include <cmath>
#include <random>

namespace ast {

float Asteroid::radius(AsteroidSize s) noexcept {
    if (s == AsteroidSize::Large)  { return kRadiusLarge; }
    if (s == AsteroidSize::Medium) { return kRadiusMedium; }
    return kRadiusSmall;
}

std::vector<Vec2> generateAsteroidShape(AsteroidSize size, int vertices, uint32_t seed,
                                        float scale) {
    constexpr float kTwoPi     = 6.28318530F;
    constexpr float kJitterFrac = 0.2F;
    const float baseRadius = Asteroid::radius(size) * scale;

    std::minstd_rand rng(seed);
    std::uniform_real_distribution<float> jitter(
        -(kJitterFrac * baseRadius),
          kJitterFrac * baseRadius);

    std::vector<Vec2> points;
    points.reserve(static_cast<std::size_t>(vertices));
    for (int i = 0; i < vertices; ++i) {
        const float a = (kTwoPi * static_cast<float>(i)) / static_cast<float>(vertices);
        const float r = baseRadius + jitter(rng);
        points.push_back({r * std::cos(a), r * std::sin(a)});
    }
    return points;
}

} // namespace ast
