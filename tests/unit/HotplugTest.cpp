#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "MockAudioSink.hpp"
#include "TestHelpers.hpp"

// Game keeps running after the controller is disconnected (connected=false).
TEST(Hotplug, GameContinuesAfterControllerDisconnect) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ASSERT_EQ(game.state(), ast::GameState::Playing);

    ast::test::tickFrames(game, 60);  // one second with no controller

    EXPECT_EQ(game.state(), ast::GameState::Playing);
}

// Ship retains velocity and continues moving (coasts) when inputs are absent.
TEST(Hotplug, ShipCoastsWhenInputsCleared) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::InputState thrustInput;
    thrustInput.start     = true;
    thrustInput.thrust    = true;
    thrustInput.connected = true;
    game.update(1.0F / 60.0F, thrustInput);

    ASSERT_GT(game.ship().velocity.length(), 0.0F);

    const ast::Vec2 posBefore = game.ship().position;
    ast::test::tickFrames(game, 1);
    const ast::Vec2 posAfter = game.ship().position;

    EXPECT_NE(posAfter.y, posBefore.y);
}

// Attract screen cannot be entered without pressing start (connected=false means start=false).
TEST(Hotplug, CannotStartGameWithoutController) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::test::tickFrames(game, 10);  // tick with connected=false, start=false

    EXPECT_EQ(game.state(), ast::GameState::Attract);
}
