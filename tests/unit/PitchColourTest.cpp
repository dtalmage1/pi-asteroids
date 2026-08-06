#include <gtest/gtest.h>
#include "pitch/PitchColour.hpp"

// Pitch classes land on exact hue-wheel boundaries every 2 semitones (60 degrees),
// giving deterministic primary/secondary RGB colours to assert against.
TEST(PitchColour, CIsRed) {
    const ast::Colour c = ast::pitchClassColour(0); // hue 0
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
}

TEST(PitchColour, DIsYellow) {
    const ast::Colour c = ast::pitchClassColour(2); // hue 60
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 0);
}

TEST(PitchColour, EIsGreen) {
    const ast::Colour c = ast::pitchClassColour(4); // hue 120
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 0);
}

TEST(PitchColour, FSharpIsCyan) {
    const ast::Colour c = ast::pitchClassColour(6); // hue 180
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 255);
    EXPECT_EQ(c.b, 255);
}

TEST(PitchColour, GSharpIsBlue) {
    const ast::Colour c = ast::pitchClassColour(8); // hue 240
    EXPECT_EQ(c.r, 0);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 255);
}

TEST(PitchColour, ASharpIsMagenta) {
    const ast::Colour c = ast::pitchClassColour(10); // hue 300
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 255);
}

// Colours are always fully opaque.
TEST(PitchColour, AlphaIsAlwaysOpaque) {
    EXPECT_EQ(ast::pitchClassColour(0).a, 255);
    EXPECT_EQ(ast::pitchClassColour(11).a, 255);
}

// Out-of-range pitch classes wrap modulo 12 rather than producing garbage.
TEST(PitchColour, NegativePitchClassWrapsToB) {
    const ast::Colour negative = ast::pitchClassColour(-1);
    const ast::Colour b        = ast::pitchClassColour(11);
    EXPECT_EQ(negative.r, b.r);
    EXPECT_EQ(negative.g, b.g);
    EXPECT_EQ(negative.b, b.b);
}

TEST(PitchColour, PitchClassTwelveWrapsToC) {
    const ast::Colour twelve = ast::pitchClassColour(12);
    const ast::Colour c      = ast::pitchClassColour(0);
    EXPECT_EQ(twelve.r, c.r);
    EXPECT_EQ(twelve.g, c.g);
    EXPECT_EQ(twelve.b, c.b);
}
