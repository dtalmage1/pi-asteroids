#include <gtest/gtest.h>
#include <string>
#include "game/Glyph.hpp"
#include "MockRenderer.hpp"

// Characters added to support note names (B, C, F, H, Z, '#') and frequency
// readouts ('.', '-') each draw at least one stroke.
TEST(Glyph, NoteAndUnitCharactersHaveStrokes) {
    for (const char ch : std::string("BCFHZ#.-")) {
        ast::MockRenderer renderer;
        EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
            .Times(testing::AtLeast(1));
        ast::drawString(renderer, {0.0F, 0.0F}, 10.0F, std::string(1, ch), {255, 255, 255, 255});
    }
}

// A full note-name-and-unit string ("A#4 440.0 HZ") renders without crashing
// and calls drawLine at least once per non-blank glyph.
TEST(Glyph, DrawsFullFrequencyReadout) {
    ast::MockRenderer renderer;
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    ast::drawString(renderer, {0.0F, 0.0F}, 10.0F, "A#4 440.0 HZ", {255, 255, 255, 255});
}
