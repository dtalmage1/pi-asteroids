#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
#include "MockAudioSink.hpp"
#include "TestHelpers.hpp"

namespace {

void doHyperspace(ast::Game& game) {
    ast::InputState input;
    input.hyperspace = true;
    game.update(1.0F / 60.0F, input);
}

} // namespace

// Hyperspace moves the ship away from the starting centre position.
// The RNG always produces a non-centre position for any seed used here.
TEST(Hyperspace, WarpChangesShipPosition) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    const ast::Vec2 before = game.ship().position;
    doHyperspace(game);

    const ast::Vec2 after = game.ship().position;
    EXPECT_TRUE((after.x != before.x) || (after.y != before.y));
}

// Ship survives the first warp (seed 1: minstd_rand(1) % 5 == 1, not 0).
TEST(Hyperspace, ShipSurvivesFirstWarp) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    doHyperspace(game);

    EXPECT_EQ(game.state(), ast::GameState::Playing);
    EXPECT_TRUE(game.ship().active);
}

// Ship survives the first four warps in a row (seeds 1–4 all non-zero mod 5).
TEST(Hyperspace, ShipSurvivesFourConsecutiveWarps) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    for (int i = 0; i < 4; ++i) { doHyperspace(game); }

    EXPECT_EQ(game.state(), ast::GameState::Playing);
    EXPECT_TRUE(game.ship().active);
}

// The fifth warp destroys the ship (seed 5: minstd_rand(5) % 5 == 0).
TEST(Hyperspace, FifthWarpKillsShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    for (int i = 0; i < 5; ++i) { doHyperspace(game); }

    EXPECT_EQ(game.state(), ast::GameState::PlayerDead);
    EXPECT_FALSE(game.ship().active);
}

// Hyperspace death decrements lives and spawns particles, same as collision.
TEST(Hyperspace, FifthWarpDecrementsLives) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    const int livesBefore = game.lives();

    for (int i = 0; i < 5; ++i) { doHyperspace(game); }

    EXPECT_EQ(game.lives(), livesBefore - 1);
}

// Warp position is within the screen bounds.
TEST(Hyperspace, WarpPositionWithinScreen) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    doHyperspace(game);

    EXPECT_GE(game.ship().position.x, 0.0F);
    EXPECT_LE(game.ship().position.x, 800.0F);
    EXPECT_GE(game.ship().position.y, 0.0F);
    EXPECT_LE(game.ship().position.y, 600.0F);
}

// Hyperspace has no effect in Attract state.
TEST(Hyperspace, IgnoredInAttractState) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    // Do NOT call startGame — stay in Attract
    doHyperspace(game);
    EXPECT_EQ(game.state(), ast::GameState::Attract);
}

// Hyperspace has no effect when the ship is inactive (PlayerDead).
TEST(Hyperspace, IgnoredWhenShipInactive) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    // Kill the ship via collision
    ast::Asteroid rock;
    rock.position   = game.ship().position;
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Large;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 999U);
    rock.active     = true;
    game.spawnAsteroid(rock);
    ast::test::tickFrames(game, 1);  // triggers collision → PlayerDead

    ASSERT_EQ(game.state(), ast::GameState::PlayerDead);
    const ast::Vec2 posBefore = game.ship().position;
    doHyperspace(game);
    EXPECT_EQ(game.ship().position.x, posBefore.x);
    EXPECT_EQ(game.ship().position.y, posBefore.y);
}
