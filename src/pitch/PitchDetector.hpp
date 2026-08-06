#pragma once
#include <optional>
#include <vector>

namespace ast {

struct PitchResult {
    float frequencyHz = 0.0F;
    float clarity     = 0.0F; // normalised autocorrelation peak, 0..1
};

// Estimates the fundamental frequency of a mono PCM buffer (samples in
// [-1, 1]) using normalised autocorrelation with parabolic peak
// interpolation. No SDL2 dependency — pure signal processing, testable with
// synthetic sine waves.
class PitchDetector {
public:
    explicit PitchDetector(int sampleRate, float minHz = 65.0F, float maxHz = 1200.0F,
                            float clarityThreshold = 0.35F);

    // Returns nullopt when the buffer is too short, silent, or not
    // sufficiently periodic (clarity below threshold) within [minHz, maxHz].
    std::optional<PitchResult> detect(const std::vector<float>& samples) const;

private:
    int   sampleRate_;
    float minHz_;
    float maxHz_;
    float clarityThreshold_;
};

} // namespace ast
