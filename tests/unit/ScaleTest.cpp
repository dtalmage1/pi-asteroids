#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
#include "MockAudioSink.hpp"
#include "TestHelpers.hpp"

// Thrust acceleration is proportional to screen height / 600.
TEST(Scale, ThrustAccelScalesWithScreenHeight) {
    ast::MockAudioSink audio;

    ast::InputState thrustInput;
    thrustInput.start     = true;
    thrustInput.thrust    = true;
    thrustInput.connected = true;

    ast::Game game600(audio, {800.0F, 600.0F});
    game600.update(1.0F / 60.0F, thrustInput);
    const float speed600 = game600.ship().velocity.length();

    ast::Game game1200(audio, {800.0F, 1200.0F});
    game1200.update(1.0F / 60.0F, thrustInput);
    const float speed1200 = game1200.ship().velocity.length();

    ASSERT_GT(speed600, 0.0F);
    EXPECT_NEAR(speed1200 / speed600, 2.0F, 0.001F);
}

// Collision radius scales: an asteroid that misses at 600 px height hits at 1200 px.
TEST(Scale, CollisionRadiusScalesWithScreenHeight) {
    ast::MockAudioSink audio;

    // At 600 px: ship kRadius=10, asteroid small kRadius=12 → sum=22.
    // Place asteroid 25 px away → no collision (25 > 22).
    ast::Game game600(audio, {800.0F, 600.0F});
    ast::test::startGame(game600);
    ast::Asteroid rock600;
    rock600.position = game600.ship().position + ast::Vec2{25.0F, 0.0F};
    rock600.velocity = {0.0F, 0.0F};
    rock600.size     = ast::AsteroidSize::Small;
    rock600.active   = true;
    game600.spawnAsteroid(rock600);
    game600.update(1.0F / 60.0F, ast::InputState{});
    EXPECT_EQ(game600.state(), ast::GameState::Playing);

    // At 1200 px: effective ship=20, asteroid small=24 → sum=44.
    // Same 25 px offset → collision (25 < 44).
    ast::Game game1200(audio, {800.0F, 1200.0F});
    ast::test::startGame(game1200);
    ast::Asteroid rock1200;
    rock1200.position = game1200.ship().position + ast::Vec2{25.0F, 0.0F};
    rock1200.velocity = {0.0F, 0.0F};
    rock1200.size     = ast::AsteroidSize::Small;
    rock1200.active   = true;
    game1200.spawnAsteroid(rock1200);
    game1200.update(1.0F / 60.0F, ast::InputState{});
    EXPECT_NE(game1200.state(), ast::GameState::Playing);
}
