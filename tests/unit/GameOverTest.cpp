#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/Glyph.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"
#include "TestHelpers.hpp"

namespace {

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

// Reach GameOver by placing a stationary kill-rock that auto-triggers three deaths.
void reachGameOver(ast::Game& game) {
    ast::test::startGame(game);
    killShip(game);
    ast::test::tickFrames(game, 1000);
}

} // namespace

// GameOver state is not entered while lives remain.
TEST(GameOver, IsNotGameOverWithLivesRemaining) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    killShip(game);
    EXPECT_NE(game.state(), ast::GameState::GameOver);
}

// State does not auto-transition before the 5-second delay.
TEST(GameOver, DoesNotTransitionBeforeDelay) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::test::tickFrames(game, 60);  // 1 more second — well within the remaining delay window
    EXPECT_EQ(game.state(), ast::GameState::GameOver);
}

// State transitions to Attract once the 5-second delay expires.
TEST(GameOver, TransitionsToAttractOnTimer) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::test::tickFrames(game, 310);  // > 5 seconds — delay expired
    EXPECT_EQ(game.state(), ast::GameState::Attract);
}

// Pressing START in GameOver returns immediately to Attract.
TEST(GameOver, TransitionsToAttractOnStart) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::InputState startInput;
    startInput.start = true;
    game.update(1.0F / 60.0F, startInput);
    EXPECT_EQ(game.state(), ast::GameState::Attract);
}

// Score is unchanged while waiting on the GameOver screen.
TEST(GameOver, ScorePreservedDuringGameOver) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    const int scoreBefore = game.score();
    ast::test::tickFrames(game, 60);
    EXPECT_EQ(game.score(), scoreBefore);
}

// Starting a new game from Attract resets the score to zero.
TEST(GameOver, ScoreResetOnNewGame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::test::tickFrames(game, 310);  // → Attract
    ast::test::startGame(game);         // → Playing
    EXPECT_EQ(game.score(), 0);
}

// Starting a new game from Attract resets lives to 3.
TEST(GameOver, LivesResetOnNewGame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::test::tickFrames(game, 310);  // → Attract
    ast::test::startGame(game);
    EXPECT_EQ(game.lives(), 3);
}

// stringWidth returns 0 for an empty string.
TEST(Glyph, StringWidthEmpty) {
    EXPECT_FLOAT_EQ(ast::stringWidth(10.0F, ""), 0.0F);
}

// stringWidth for a single character equals the character width (0.6 × charHeight).
TEST(Glyph, StringWidthSingleChar) {
    EXPECT_FLOAT_EQ(ast::stringWidth(10.0F, "A"), 6.0F);
}

// stringWidth for two characters is advance + charWidth (no trailing gap).
TEST(Glyph, StringWidthTwoChars) {
    // 2 × 0.8h − 0.2h = 1.4h
    EXPECT_FLOAT_EQ(ast::stringWidth(10.0F, "GO"), 14.0F);
}

// drawString calls drawLine for a known character with strokes.
TEST(Glyph, DrawStringCallsDrawLine) {
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    ast::drawString(renderer, {0.0F, 0.0F}, 10.0F, "A", {255, 255, 255, 255});
}

// drawString does not call drawLine for an unrecognised character (space).
TEST(Glyph, DrawStringSkipsUnknownChar) {
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(0);
    ast::drawString(renderer, {0.0F, 0.0F}, 10.0F, " ", {255, 255, 255, 255});
}
