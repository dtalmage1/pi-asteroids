#pragma once

namespace ast {

// Returns the number of large asteroids to spawn for the given wave number (1-based).
// Starts at 4, grows by 2 per wave, capped at 11.
int waveAsteroidCount(int waveNumber) noexcept;

} // namespace ast
