#include "pitch/PitchColour.hpp"
#include <array>
#include <cmath>
#include <cstdint>

namespace {

// Converts fully-saturated HSV (hue in degrees [0,360), s=v=1) to RGB.
ast::Colour hsvToRgb(float hueDegrees) {
    static constexpr float kSextant = 60.0F;
    static constexpr float kMaxByte = 255.0F;

    const float h = std::fmod(hueDegrees, 360.0F) / kSextant;
    const auto  sextantIndex = static_cast<int>(h);
    const float fractional   = h - static_cast<float>(sextantIndex);

    const auto p = static_cast<std::uint8_t>(0.0F);
    const auto q = static_cast<std::uint8_t>((1.0F - fractional) * kMaxByte);
    const auto t = static_cast<std::uint8_t>(fractional * kMaxByte);
    const auto v = static_cast<std::uint8_t>(kMaxByte);

    switch (sextantIndex % 6) {
        case 0:  return ast::Colour{v, t, p, 255};
        case 1:  return ast::Colour{q, v, p, 255};
        case 2:  return ast::Colour{p, v, t, 255};
        case 3:  return ast::Colour{p, q, v, 255};
        case 4:  return ast::Colour{t, p, v, 255};
        default: return ast::Colour{v, p, q, 255};
    }
}

} // namespace

namespace ast {

Colour pitchClassColour(int pitchClass) {
    static constexpr int   kSemitones      = 12;
    static constexpr float kDegreesPerNote = 360.0F / static_cast<float>(kSemitones);

    const int normalised = ((pitchClass % kSemitones) + kSemitones) % kSemitones;
    return hsvToRgb(static_cast<float>(normalised) * kDegreesPerNote);
}

} // namespace ast
