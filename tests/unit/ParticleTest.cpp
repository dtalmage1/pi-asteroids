#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <algorithm>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
#include "game/entities/Particle.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"

namespace {

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

void killShip(ast::Game& game) {
    ast::Asteroid rock;
    rock.position   = game.ship().position;
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Large;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 999U);
    rock.active     = true;
    game.spawnAsteroid(rock);
    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);
}

} // namespace

// Destroying an asteroid spawns active particles.
TEST(Particle, SpawnOnAsteroidDestroy) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    // Place a Small asteroid just ahead of the ship's nose so one fire frame hits it.
    ast::Asteroid rock;
    rock.position   = {400.0F, 270.0F};
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Small;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Small, 8, 42U);
    rock.active     = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    const auto& parts = game.particles();
    EXPECT_TRUE(std::any_of(parts.begin(), parts.end(),
                             [](const ast::Particle& p) { return p.active; }));
}

// Killing the ship spawns active particles.
TEST(Particle, SpawnOnShipDestroy) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);

    const auto& parts = game.particles();
    EXPECT_TRUE(std::any_of(parts.begin(), parts.end(),
                             [](const ast::Particle& p) { return p.active; }));
}

// All particles deactivate once their lifetime expires.
TEST(Particle, ParticlesExpireOverTime) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);

    // Tick for longer than kParticleLifetimeMax (1.0 s) — 70 frames ≈ 1.17 s
    const ast::InputState noInput;
    for (int i = 0; i < 70; ++i) { game.update(1.0F / 60.0F, noInput); }

    const auto& parts = game.particles();
    EXPECT_FALSE(std::any_of(parts.begin(), parts.end(),
                              [](const ast::Particle& p) { return p.active; }));
}

// Active particles produce drawLine calls during rendering.
TEST(Particle, ParticlesRendered) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);

    ast::MockRenderer renderer;
    // Score HUD "0" = 4 strokes; 12 ship particles add 12 more — well above 4.
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(5));
    game.render(renderer);
}

// Particle positions change each frame.
TEST(Particle, ParticlesMoveOverTime) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);  // particles spawn at ship centre; killShip already ticked once

    // After the kill-frame tick, all particles should have left the spawn point.
    const auto& parts = game.particles();
    bool anyMoved = false;
    for (const auto& p : parts) {
        if (!p.active) { continue; }
        if ((p.position.x != game.ship().position.x) ||
            (p.position.y != game.ship().position.y)) {
            anyMoved = true;
            break;
        }
    }
    EXPECT_TRUE(anyMoved);
}

// No particles are active immediately after game start.
TEST(Particle, NoParticlesAtGameStart) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    const auto& parts = game.particles();
    EXPECT_FALSE(std::any_of(parts.begin(), parts.end(),
                              [](const ast::Particle& p) { return p.active; }));
}
