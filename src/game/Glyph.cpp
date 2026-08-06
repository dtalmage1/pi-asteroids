#include "game/Glyph.hpp"
#include <array>
#include <vector>

namespace {

// Each segment is {x0, y0, x1, y1} in normalised [0,1]×[0,1] space,
// where (0,0) is the top-left of the character cell.
using Seg     = std::array<float, 4>;
using Strokes = std::vector<Seg>;

// Digit and letter definitions.  Only the characters used by the game are
// defined; everything else renders as blank.
const Strokes& getGlyphStrokes(char c) {  // NOLINT(readability-function-cognitive-complexity)
    static const Strokes kEmpty;

    // Digits
    static const Strokes k0{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F},
                              {1.0F,1.0F,0.0F,1.0F},{0.0F,1.0F,0.0F,0.0F}}};
    static const Strokes k1{{{0.5F,0.0F,0.5F,1.0F}}};
    static const Strokes k2{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,0.5F},
                              {1.0F,0.5F,0.0F,0.5F},{0.0F,0.5F,0.0F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes k3{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F},
                              {0.0F,0.5F,1.0F,0.5F},{0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes k4{{{0.0F,0.0F,0.0F,0.5F},{0.0F,0.5F,1.0F,0.5F},
                              {1.0F,0.0F,1.0F,1.0F}}};
    static const Strokes k5{{{1.0F,0.0F,0.0F,0.0F},{0.0F,0.0F,0.0F,0.5F},
                              {0.0F,0.5F,1.0F,0.5F},{1.0F,0.5F,1.0F,1.0F},
                              {1.0F,1.0F,0.0F,1.0F}}};
    static const Strokes k6{{{1.0F,0.0F,0.0F,0.0F},{0.0F,0.0F,0.0F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F},{1.0F,1.0F,1.0F,0.5F},
                              {1.0F,0.5F,0.0F,0.5F}}};
    static const Strokes k7{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F}}};
    static const Strokes k8{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F},
                              {0.0F,0.0F,0.0F,1.0F},{0.0F,0.5F,1.0F,0.5F},
                              {0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes k9{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F},
                              {0.0F,0.0F,0.0F,0.5F},{0.0F,0.5F,1.0F,0.5F}}};

    static const std::array<const Strokes*, 10> kDigits{{
        &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9
    }};

    // Letters
    static const Strokes kA{{{0.0F,1.0F,0.5F,0.0F},{0.5F,0.0F,1.0F,1.0F},
                              {0.15F,0.5F,0.85F,0.5F}}};
    static const Strokes kB{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,0.6F,0.0F},
                              {0.6F,0.0F,0.9F,0.25F},{0.9F,0.25F,0.6F,0.5F},
                              {0.6F,0.5F,0.9F,0.75F},{0.9F,0.75F,0.6F,1.0F},
                              {0.6F,1.0F,0.0F,1.0F}}};
    static const Strokes kC{{{1.0F,0.0F,0.0F,0.0F},{0.0F,0.0F,0.0F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes kD{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,0.7F,0.0F},
                              {0.7F,0.0F,1.0F,0.3F},{1.0F,0.3F,1.0F,0.7F},
                              {1.0F,0.7F,0.7F,1.0F},{0.7F,1.0F,0.0F,1.0F}}};
    static const Strokes kE{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,1.0F,0.0F},
                              {0.0F,0.5F,0.7F,0.5F},{0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes kF{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,1.0F,0.0F},
                              {0.0F,0.5F,0.7F,0.5F}}};
    static const Strokes kG{{{1.0F,0.0F,0.0F,0.0F},{0.0F,0.0F,0.0F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F},{1.0F,1.0F,1.0F,0.5F},
                              {0.5F,0.5F,1.0F,0.5F}}};
    static const Strokes kH{{{0.0F,0.0F,0.0F,1.0F},{1.0F,0.0F,1.0F,1.0F},
                              {0.0F,0.5F,1.0F,0.5F}}};
    static const Strokes kI{{{0.0F,0.0F,1.0F,0.0F},{0.5F,0.0F,0.5F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F}}};
    static const Strokes kM{{{0.0F,1.0F,0.0F,0.0F},{0.0F,0.0F,0.5F,0.5F},
                              {0.5F,0.5F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F}}};
    static const Strokes kO{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,1.0F,1.0F},
                              {1.0F,1.0F,0.0F,1.0F},{0.0F,1.0F,0.0F,0.0F}}};
    static const Strokes kP{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,1.0F,0.0F},
                              {1.0F,0.0F,1.0F,0.5F},{1.0F,0.5F,0.0F,0.5F}}};
    static const Strokes kR{{{0.0F,0.0F,0.0F,1.0F},{0.0F,0.0F,1.0F,0.0F},
                              {1.0F,0.0F,1.0F,0.5F},{1.0F,0.5F,0.0F,0.5F},
                              {0.5F,0.5F,1.0F,1.0F}}};
    static const Strokes kS{{{1.0F,0.0F,0.0F,0.0F},{0.0F,0.0F,0.0F,0.5F},
                              {0.0F,0.5F,1.0F,0.5F},{1.0F,0.5F,1.0F,1.0F},
                              {1.0F,1.0F,0.0F,1.0F}}};
    static const Strokes kT{{{0.0F,0.0F,1.0F,0.0F},{0.5F,0.0F,0.5F,1.0F}}};
    static const Strokes kV{{{0.0F,0.0F,0.5F,1.0F},{0.5F,1.0F,1.0F,0.0F}}};
    static const Strokes kZ{{{0.0F,0.0F,1.0F,0.0F},{1.0F,0.0F,0.0F,1.0F},
                              {0.0F,1.0F,1.0F,1.0F}}};

    // Punctuation
    static const Strokes kHash{{{0.35F,0.0F,0.35F,1.0F},{0.65F,0.0F,0.65F,1.0F},
                                 {0.0F,0.35F,1.0F,0.35F},{0.0F,0.65F,1.0F,0.65F}}};
    static const Strokes kPeriod{{{0.45F,0.9F,0.55F,1.0F}}};
    static const Strokes kHyphen{{{0.0F,0.5F,1.0F,0.5F}}};

    if (c >= '0' && c <= '9') {
        return *kDigits.at(static_cast<std::size_t>(c - '0'));
    }
    if (c == 'A') { return kA; }
    if (c == 'B') { return kB; }
    if (c == 'C') { return kC; }
    if (c == 'D') { return kD; }
    if (c == 'E') { return kE; }
    if (c == 'F') { return kF; }
    if (c == 'G') { return kG; }
    if (c == 'H') { return kH; }
    if (c == 'I') { return kI; }
    if (c == 'M') { return kM; }
    if (c == 'O') { return kO; }
    if (c == 'P') { return kP; }
    if (c == 'R') { return kR; }
    if (c == 'S') { return kS; }
    if (c == 'T') { return kT; }
    if (c == 'V') { return kV; }
    if (c == 'Z') { return kZ; }
    if (c == '#') { return kHash; }
    if (c == '.') { return kPeriod; }
    if (c == '-') { return kHyphen; }
    return kEmpty;
}

} // namespace

namespace ast {

void drawString(IRenderer& renderer, Vec2 origin, float charHeight,
                std::string_view text, Colour colour) {
    const float charW   = charHeight * 0.6F;
    const float advance = charHeight * 0.8F;
    float xCursor = origin.x;
    for (const char ch : text) {
        for (const auto& seg : getGlyphStrokes(ch)) {
            renderer.drawLine(
                Vec2{xCursor + (seg.at(0) * charW),  origin.y + (seg.at(1) * charHeight)},
                Vec2{xCursor + (seg.at(2) * charW),  origin.y + (seg.at(3) * charHeight)},
                colour);
        }
        xCursor += advance;
    }
}

float stringWidth(float charHeight, std::string_view text) noexcept {
    if (text.empty()) { return 0.0F; }
    const float advance = charHeight * 0.8F;
    const float charW   = charHeight * 0.6F;
    return ((static_cast<float>(text.size()) * advance) - (advance - charW));
}

} // namespace ast
