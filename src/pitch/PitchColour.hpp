#pragma once
#include "game/Colour.hpp"

namespace ast {

// Maps a pitch class (0=C .. 11=B) to a fully-saturated colour, evenly
// spaced around the hue wheel (30 degrees per semitone) so each of the
// twelve notes is visually distinct. Pure colour math, no rendering
// dependency.
Colour pitchClassColour(int pitchClass);

} // namespace ast
