#include "pitch/NoteUtils.hpp"
#include <array>
#include <cmath>
#include <cstddef>

namespace {

constexpr std::array<const char*, 12> kNoteNames{
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

} // namespace

namespace ast {

Note frequencyToNote(float frequencyHz) {
    if (frequencyHz <= 0.0F) { return Note{}; }

    static constexpr float kA4Freq      = 440.0F;
    static constexpr int   kA4Midi      = 69;
    static constexpr int   kSemitones   = 12;
    static constexpr float kCentsPerSemitone = 100.0F;

    const float midiFloat = static_cast<float>(kA4Midi)
                           + (static_cast<float>(kSemitones) * std::log2(frequencyHz / kA4Freq));
    const int   midi      = static_cast<int>(std::lround(midiFloat));
    const float cents     = (midiFloat - static_cast<float>(midi)) * kCentsPerSemitone;

    const int pitchClass = ((midi % kSemitones) + kSemitones) % kSemitones;
    const int octave     = (midi / kSemitones) - 1;

    Note note;
    note.midiNumber  = midi;
    note.pitchClass  = pitchClass;
    note.centsOffset = cents;
    note.name        = std::string(kNoteNames.at(static_cast<std::size_t>(pitchClass)))
                      + std::to_string(octave);
    return note;
}

} // namespace ast
