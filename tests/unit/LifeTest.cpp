#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "MockAudioSink.hpp"

namespace {

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

void tickFrames(ast::Game& game, int n) {
    const ast::InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
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

// Lives is 3 immediately after game starts.
TEST(Life, StartsWithThreeLives) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    EXPECT_EQ(game.lives(), 3);
}

// A fatal collision decrements the live count.
TEST(Life, DeathDecrementsLives) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    EXPECT_EQ(game.lives(), 2);
}

// Ship is inactive immediately after a fatal collision.
TEST(Life, ShipInactiveAfterDeath) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    EXPECT_FALSE(game.ship().active);
}

// State transitions to PlayerDead on fatal collision.
TEST(Life, StateIsPlayerDeadAfterDeath) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    EXPECT_EQ(game.state(), ast::GameState::PlayerDead);
}

// Respawn does not occur before the 3-second delay has elapsed.
TEST(Life, NoRespawnBeforeDelay) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    tickFrames(game, 60);  // 1 second — well before the 3-second respawn
    EXPECT_EQ(game.state(), ast::GameState::PlayerDead);
}

// Ship respawns to active once the 3-second delay has passed.
TEST(Life, ShipRespawnsAfterDelay) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    tickFrames(game, 185);  // > 3 seconds — respawn guaranteed
    EXPECT_TRUE(game.ship().active);
}

// Respawned ship has invincibility timer > 0.
TEST(Life, RespawnedShipIsInvincible) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    tickFrames(game, 185);
    EXPECT_GT(game.ship().invincTimer, 0.0F);
}

// Respawned ship appears at screen centre.
TEST(Life, RespawnedShipAtCenter) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    tickFrames(game, 185);
    EXPECT_FLOAT_EQ(game.ship().position.x, 400.0F);
    EXPECT_FLOAT_EQ(game.ship().position.y, 300.0F);
}

// Invincible ship is not destroyed by an overlapping asteroid.
TEST(Life, InvincShipSurvivesCollision) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);                // kill-rock placed at centre (400, 300)
    tickFrames(game, 185);         // respawned at centre with invincTimer > 0
    tickFrames(game, 1);           // kill-rock still at centre; invincTimer still > 0
    EXPECT_TRUE(game.ship().active);
    EXPECT_EQ(game.state(), ast::GameState::Playing);
}

// All three lives are exhausted and state reaches GameOver.
TEST(Life, GameOverWhenLivesExhausted) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);  // death 1; stationary kill-rock remains at (400, 300)
    // The kill-rock auto-triggers deaths 2 and 3 each time invincibility
    // expires after each respawn (~360 frames per cycle × 2, plus a final
    // ~180-frame respawn-to-GameOver transition ≈ 900 frames total).
    tickFrames(game, 1000);
    EXPECT_EQ(game.state(), ast::GameState::GameOver);
}
