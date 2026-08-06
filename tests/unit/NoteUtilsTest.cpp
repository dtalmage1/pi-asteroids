#include <gtest/gtest.h>
#include "pitch/NoteUtils.hpp"

// 440 Hz is the concert pitch reference: exactly A4, zero cents deviation.
TEST(NoteUtils, ConcertPitchIsA4) {
    const ast::Note note = ast::frequencyToNote(440.0F);
    EXPECT_EQ(note.name, "A4");
    EXPECT_EQ(note.midiNumber, 69);
    EXPECT_EQ(note.pitchClass, 9);
    EXPECT_NEAR(note.centsOffset, 0.0F, 0.01F);
}

// Middle C (261.626 Hz) is C4 in scientific pitch notation.
TEST(NoteUtils, MiddleCIsC4) {
    const ast::Note note = ast::frequencyToNote(261.6256F);
    EXPECT_EQ(note.name, "C4");
    EXPECT_EQ(note.pitchClass, 0);
}

// One octave above concert pitch is A5.
TEST(NoteUtils, OctaveAboveConcertPitchIsA5) {
    const ast::Note note = ast::frequencyToNote(880.0F);
    EXPECT_EQ(note.name, "A5");
}

// A sharp note reports its '#' in the name.
TEST(NoteUtils, SharpNoteIncludesHash) {
    const ast::Note note = ast::frequencyToNote(415.3047F); // G#4 / Ab4
    EXPECT_EQ(note.name, "G#4");
}

// A frequency slightly above a note's exact pitch reports a positive cents offset.
TEST(NoteUtils, SharpFrequencyHasPositiveCents) {
    const ast::Note note = ast::frequencyToNote(445.0F); // ~20 cents sharp of A4
    EXPECT_EQ(note.midiNumber, 69);
    EXPECT_NEAR(note.centsOffset, 19.6F, 0.2F);
}

// A frequency slightly below a note's exact pitch reports a negative cents offset.
TEST(NoteUtils, FlatFrequencyHasNegativeCents) {
    const ast::Note note = ast::frequencyToNote(435.0F); // ~20 cents flat of A4
    EXPECT_EQ(note.midiNumber, 69);
    EXPECT_LT(note.centsOffset, 0.0F);
}

// Non-positive frequencies (e.g. detector edge cases) yield a default Note rather than
// crashing on log2(0) or log2(negative).
TEST(NoteUtils, NonPositiveFrequencyReturnsDefaultNote) {
    EXPECT_EQ(ast::frequencyToNote(0.0F).name, "");
    EXPECT_EQ(ast::frequencyToNote(-10.0F).name, "");
}
