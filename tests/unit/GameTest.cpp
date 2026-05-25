#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"

// Game starts in Attract state immediately after construction.
TEST(Game, StartsInAttract) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_EQ(game.state(), ast::GameState::Attract);
}

// update() is callable without side effects while the state machine is a stub.
TEST(Game, UpdateDoesNotCrash) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    const ast::InputState input;
    game.update(1.0F / 60.0F, input);
    EXPECT_EQ(game.state(), ast::GameState::Attract);
}

// render() draws the ship wireframe once per frame.
TEST(Game, RenderDrawsShipWireframe) {
    ast::MockAudioSink audio;
    ast::MockRenderer  renderer;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, true)).Times(1);
    game.render(renderer);
}

// Ship spawns at the centre of the screen on construction.
TEST(Game, ShipSpawnsAtScreenCenter) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_FLOAT_EQ(game.ship().position.x, 400.0F);
    EXPECT_FLOAT_EQ(game.ship().position.y, 300.0F);
}

// START button in Attract state transitions to Playing.
TEST(Game, StartTransitionsToPlaying) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
    EXPECT_EQ(game.state(), ast::GameState::Playing);
}

// Rotate-left input decreases ship angle in Playing state.
TEST(Game, RotateLeftDecreasesAngle) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::InputState start;
    start.start = true;
    game.update(1.0F / 60.0F, start);
    const float angleBefore = game.ship().angle;
    ast::InputState input;
    input.rotateLeft = true;
    game.update(1.0F / 60.0F, input);
    // angle wraps through 2π so compare via modular decrease
    const float delta = game.ship().angle - angleBefore;
    EXPECT_TRUE(delta < 0.0F || delta > 6.0F); // decreased or wrapped
}

// Rotate-right input increases ship angle in Playing state.
TEST(Game, RotateRightIncreasesAngle) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::InputState start;
    start.start = true;
    game.update(1.0F / 60.0F, start);
    ast::InputState input;
    input.rotateRight = true;
    game.update(1.0F / 60.0F, input);
    EXPECT_GT(game.ship().angle, 0.0F);
}

// Thrust input accelerates the ship.
TEST(Game, ThrustAcceleratesShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::InputState start;
    start.start = true;
    game.update(1.0F / 60.0F, start);
    ast::InputState input;
    input.thrust = true;
    game.update(1.0F / 60.0F, input);
    EXPECT_GT(game.ship().velocity.length(), 0.0F);
}

// ship.thrusting flag tracks the thrust input each frame.
TEST(Game, ThrustFlagTracksInput) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::InputState start;
    start.start = true;
    game.update(1.0F / 60.0F, start);
    ast::InputState thrustOn;
    thrustOn.thrust = true;
    game.update(1.0F / 60.0F, thrustOn);
    EXPECT_TRUE(game.ship().thrusting);
    ast::InputState thrustOff;
    game.update(1.0F / 60.0F, thrustOff);
    EXPECT_FALSE(game.ship().thrusting);
}
