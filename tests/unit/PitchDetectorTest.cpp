#include <gtest/gtest.h>
#include <cmath>
#include <cstddef>
#include <vector>
#include "pitch/PitchDetector.hpp"

namespace {

std::vector<float> makeSineWave(float freqHz, int sampleRate, std::size_t sampleCount) {
    static constexpr float kAmplitude = 0.8F;
    static constexpr float kTwoPi     = 6.28318530718F;
    std::vector<float> samples(sampleCount);
    for (std::size_t i = 0; i < sampleCount; ++i) {
        samples[i] = kAmplitude
                   * std::sin(kTwoPi * freqHz * static_cast<float>(i) / static_cast<float>(sampleRate));
    }
    return samples;
}

constexpr int kSampleRate = 48000;

} // namespace

// A clean 440 Hz sine wave (concert A) is detected within 2 Hz.
TEST(PitchDetector, DetectsA4SineWave) {
    const ast::PitchDetector detector(kSampleRate);
    const auto result = detector.detect(makeSineWave(440.0F, kSampleRate, 4096));
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access) guarded by ASSERT_TRUE above
    EXPECT_NEAR(result->frequencyHz, 440.0F, 2.0F);
}

// A low male-voice-range tone (110 Hz) is detected within 1 Hz.
TEST(PitchDetector, DetectsLowFrequencySineWave) {
    const ast::PitchDetector detector(kSampleRate);
    const auto result = detector.detect(makeSineWave(110.0F, kSampleRate, 4096));
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access) guarded by ASSERT_TRUE above
    EXPECT_NEAR(result->frequencyHz, 110.0F, 1.0F);
}

// A high female-voice-range tone (880 Hz) is detected within 4 Hz.
TEST(PitchDetector, DetectsHighFrequencySineWave) {
    const ast::PitchDetector detector(kSampleRate);
    const auto result = detector.detect(makeSineWave(880.0F, kSampleRate, 4096));
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access) guarded by ASSERT_TRUE above
    EXPECT_NEAR(result->frequencyHz, 880.0F, 4.0F);
}

// Silence (all zeros) has no periodicity to lock onto.
TEST(PitchDetector, SilenceReturnsNullopt) {
    const ast::PitchDetector detector(kSampleRate);
    const std::vector<float> samples(4096, 0.0F);
    EXPECT_FALSE(detector.detect(samples).has_value());
}

// A buffer shorter than twice the maximum lag cannot be analysed.
TEST(PitchDetector, TooShortBufferReturnsNullopt) {
    const ast::PitchDetector detector(kSampleRate);
    EXPECT_FALSE(detector.detect(makeSineWave(440.0F, kSampleRate, 100)).has_value());
}

// A clean sine wave produces a near-perfect (close to 1.0) clarity score.
TEST(PitchDetector, ClarityIsHighForCleanSineWave) {
    const ast::PitchDetector detector(kSampleRate);
    const auto result = detector.detect(makeSineWave(440.0F, kSampleRate, 4096));
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access) guarded by ASSERT_TRUE above
    EXPECT_GT(result->clarity, 0.9F);
}

// An inverted [minHz, maxHz] range yields an empty lag search window, so no
// lag is ever evaluated and the detector reports no pitch.
TEST(PitchDetector, InvertedFrequencyRangeReturnsNullopt) {
    const ast::PitchDetector detector(kSampleRate, 1200.0F, 65.0F);
    const auto result = detector.detect(makeSineWave(440.0F, kSampleRate, 4096));
    EXPECT_FALSE(result.has_value());
}
