#pragma once
#include <string>

namespace ast {

struct Note {
    std::string name;         // e.g. "A4", "C#3"
    int         midiNumber = 0;
    int         pitchClass = 0; // 0=C, 1=C#, ... 11=B
    float       centsOffset = 0.0F; // deviation from the nearest note, -50..+50
};

// Converts a frequency in Hz to the nearest equal-tempered note, using
// A4 = 440 Hz as the reference pitch. No SDL2 dependency; pure math.
Note frequencyToNote(float frequencyHz);

} // namespace ast
