#include "pitch/PitchDetector.hpp"
#include <cstddef>

namespace {

float mean(const std::vector<float>& samples) {
    float sum = 0.0F;
    for (const float s : samples) { sum += s; }
    return samples.empty() ? 0.0F : sum / static_cast<float>(samples.size());
}

std::vector<float> removeDcOffset(const std::vector<float>& samples) {
    const float m = mean(samples);
    std::vector<float> centred(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) {
        centred.at(i) = samples.at(i) - m;
    }
    return centred;
}

// Autocorrelation at a single lag, normalised by the zero-lag energy.
float normalisedAutocorrelation(const std::vector<float>& centred, int lag, float energy0) {
    float sum = 0.0F;
    const auto n = centred.size();
    for (std::size_t i = 0; i + static_cast<std::size_t>(lag) < n; ++i) {
        sum += centred.at(i) * centred.at(i + static_cast<std::size_t>(lag));
    }
    return sum / energy0;
}

// Refines an integer lag to sub-sample precision using parabolic
// interpolation over the correlation values at lag-1, lag, lag+1.
float refineLag(const std::vector<float>& centred, int lag, float energy0) {
    const float corrPrev = normalisedAutocorrelation(centred, lag - 1, energy0);
    const float corrHere = normalisedAutocorrelation(centred, lag, energy0);
    const float corrNext = normalisedAutocorrelation(centred, lag + 1, energy0);

    const float denom = (corrPrev - (2.0F * corrHere) + corrNext);
    if (denom == 0.0F) { return static_cast<float>(lag); }

    const float offset = 0.5F * (corrPrev - corrNext) / denom;
    return static_cast<float>(lag) + offset;
}

} // namespace

namespace ast {

PitchDetector::PitchDetector(int sampleRate, float minHz, float maxHz, float clarityThreshold)
    : sampleRate_(sampleRate)
    , minHz_(minHz)
    , maxHz_(maxHz)
    , clarityThreshold_(clarityThreshold)
{
}

std::optional<PitchResult> PitchDetector::detect(const std::vector<float>& samples) const {
    const int minLag = static_cast<int>(static_cast<float>(sampleRate_) / maxHz_);
    const int maxLag = static_cast<int>(static_cast<float>(sampleRate_) / minHz_);

    if (minLag < 1 || samples.size() < static_cast<std::size_t>(maxLag) * 2) {
        return std::nullopt;
    }

    const std::vector<float> centred = removeDcOffset(samples);

    float energy0 = 0.0F;
    for (const float s : centred) { energy0 += s * s; }
    static constexpr float kSilenceEnergy = 1e-6F;
    if (energy0 < kSilenceEnergy) { return std::nullopt; }

    float bestCorr = -1.0F;
    int   bestLag  = 0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        const float corr = normalisedAutocorrelation(centred, lag, energy0);
        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag  = lag;
        }
    }

    if (bestLag <= 0 || bestCorr < clarityThreshold_) { return std::nullopt; }

    const float refinedLag = (bestLag > minLag && bestLag < maxLag)
                                  ? refineLag(centred, bestLag, energy0)
                                  : static_cast<float>(bestLag);
    if (refinedLag <= 0.0F) { return std::nullopt; }

    return PitchResult{static_cast<float>(sampleRate_) / refinedLag, bestCorr};
}

} // namespace ast
