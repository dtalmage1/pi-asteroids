#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/Glyph.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"

namespace {

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

} // namespace

// "ASTEROIDS" title and "PRESS START" prompt produce many drawLine calls in Attract.
TEST(Attract, TextRenderedInAttract) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(20));  // title alone has 30+ strokes
    game.render(renderer);
}

// Attract overlay is not rendered during Playing state.
TEST(Attract, TextNotRenderedWhilePlaying) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    ast::MockRenderer renderer;
    // Score HUD "0" = 4 strokes; attract title would add 30+
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::Between(1, 10));
    game.render(renderer);
}

// stringWidth is correct for a 9-character string.
TEST(Attract, StringWidthAsteroids) {
    // (9 × 0.8h) − (0.8h − 0.6h) = 7.2h − 0.2h = 7.0h
    EXPECT_FLOAT_EQ(ast::stringWidth(10.0F, "ASTEROIDS"), 70.0F);
}

// stringWidth is correct for "PRESS START" (10 glyphs + 1 space = 11 char positions).
TEST(Attract, StringWidthPressStart) {
    // (11 × 0.8h) − (0.8h − 0.6h) = 8.8h − 0.2h = 8.6h
    EXPECT_FLOAT_EQ(ast::stringWidth(10.0F, "PRESS START"), 86.0F);
}

// New glyphs D, I, P, S, T all produce strokes.
TEST(Attract, NewGlyphsRender) {
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    ast::drawString(renderer, {0.0F, 0.0F}, 10.0F, "STIPD", {255, 255, 255, 255});
}
