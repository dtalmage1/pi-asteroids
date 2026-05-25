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
