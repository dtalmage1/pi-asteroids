#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
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

// Score digits are drawn (via drawLine) during the Playing state.
TEST(Hud, ScoreRenderedWhilePlaying) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    game.render(renderer);
}

// No score digits are drawn in the Attract state.
TEST(Hud, ScoreNotRenderedInAttract) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(0);
    game.render(renderer);
}

// Playing state with 3 lives: ship (1) + 3 life icons = 4 drawLineStrip calls.
TEST(Hud, LivesIconsRenderedWhilePlaying) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, testing::_))
        .Times(4);
    game.render(renderer);
}

// Attract state: only the demo ship is drawn — no life icons.
TEST(Hud, LivesIconsNotRenderedInAttract) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, testing::_))
        .Times(1);  // demo ship only
    game.render(renderer);
}

// After one death: lives_ == 2, ship inactive → kill-rock (1) + 2 icons = 3 drawLineStrip calls.
TEST(Hud, LivesIconCountMatchesLives) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    killShip(game);
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, testing::_))
        .Times(3);
    game.render(renderer);
}
