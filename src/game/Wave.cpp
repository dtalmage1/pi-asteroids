#include "game/Wave.hpp"
#include <algorithm>

namespace {
constexpr int kFirstWaveCount = 4;
constexpr int kWaveCountStep  = 2;
constexpr int kMaxWaveCount   = 11;
} // namespace

namespace ast {

int waveAsteroidCount(int waveNumber) noexcept {
    const int count = kFirstWaveCount + ((waveNumber - 1) * kWaveCountStep);
    return std::min(count, kMaxWaveCount);
}

} // namespace ast
