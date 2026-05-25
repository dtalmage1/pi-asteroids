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

// render() is callable without drawing anything while it is a stub.
TEST(Game, RenderDoesNotCrash) {
    ast::MockAudioSink audio;
    ast::MockRenderer  renderer;
    ast::Game game(audio, {800.0F, 600.0F});
    game.render(renderer);
}
