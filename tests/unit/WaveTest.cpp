#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/Wave.hpp"
#include "MockAudioSink.hpp"

namespace {
void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

int countActive(const std::vector<ast::Asteroid>& rocks) {
    int n = 0;
    for (const auto& r : rocks) { if (r.active) { ++n; } }
    return n;
}
} // namespace

// Wave 1 spawns 4 large asteroids.
TEST(Wave, AsteroidCountWaveOne) {
    EXPECT_EQ(ast::waveAsteroidCount(1), 4);
}

// Wave 2 spawns 6 large asteroids.
TEST(Wave, AsteroidCountWaveTwo) {
    EXPECT_EQ(ast::waveAsteroidCount(2), 6);
}

// Count grows by 2 each wave.
TEST(Wave, AsteroidCountIncreasesEachWave) {
    EXPECT_EQ(ast::waveAsteroidCount(3), 8);
    EXPECT_EQ(ast::waveAsteroidCount(4), 10);
}

// Count is capped at 11 and does not exceed it.
TEST(Wave, AsteroidCountCappedAtMax) {
    EXPECT_EQ(ast::waveAsteroidCount(5), 11);
    EXPECT_EQ(ast::waveAsteroidCount(10), 11);
    EXPECT_EQ(ast::waveAsteroidCount(100), 11);
}

// Wave number is 0 before the game starts.
TEST(GameWave, WaveNumberZeroBeforeGameStart) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_EQ(game.waveNumber(), 0);
}

// No asteroids spawn immediately after pressing start.
TEST(GameWave, NoAsteroidsImmediatelyAfterStart) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    EXPECT_EQ(countActive(game.asteroids()), 0);
}

// Startup delay prevents wave spawn before 2 seconds have elapsed.
TEST(GameWave, WaveTimerDelaysFirstSpawn) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    const ast::InputState noInput;
    for (int i = 0; i < 60; ++i) { game.update(1.0F / 60.0F, noInput); }
    EXPECT_EQ(countActive(game.asteroids()), 0);
}

// After the startup delay, wave 1 spawns 4 large asteroids and waveNumber becomes 1.
TEST(GameWave, WaveOneSpawnsFourLargeAsteroids) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    const ast::InputState noInput;
    for (int i = 0; i < 130; ++i) { game.update(1.0F / 60.0F, noInput); }

    EXPECT_EQ(game.waveNumber(), 1);
    EXPECT_EQ(countActive(game.asteroids()), 4);
    for (const auto& rock : game.asteroids()) {
        if (rock.active) { EXPECT_EQ(rock.size, ast::AsteroidSize::Large); }
    }
}

// Wave asteroids spawn at the screen edges, well clear of the ship at centre.
TEST(GameWave, AsteroidsSpawnClearOfShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    const ast::InputState noInput;
    for (int i = 0; i < 130; ++i) { game.update(1.0F / 60.0F, noInput); }

    const ast::Vec2 shipPos = game.ship().position;
    for (const auto& rock : game.asteroids()) {
        if (!rock.active) { continue; }
        const ast::Vec2 diff{rock.position.x - shipPos.x, rock.position.y - shipPos.y};
        EXPECT_GE(diff.length(), 100.0F);
    }
}
