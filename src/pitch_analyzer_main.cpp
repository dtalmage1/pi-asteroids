// Singing frequency analyzer: a standalone tool (separate from the asteroids
// binary) that listens to the microphone and displays the detected pitch as
// a number and as a scrolling, note-coloured graph.
#define SDL_MAIN_HANDLED // prevent SDL from redefining main as SDL_main
#include "platform/Platform.hpp"
#include "rendering/Sdl2Renderer.hpp"
#include "audio/Sdl2AudioCapture.hpp"
#include "pitch/IAudioCapture.hpp"
#include "pitch/PitchDetector.hpp"
#include "pitch/NoteUtils.hpp"
#include "pitch/PitchColour.hpp"
#include "game/Glyph.hpp"
#include "game/Colour.hpp"
#include "game/IRenderer.hpp"
#include "game/Vec2.hpp"
#include <SDL.h>
#include <cmath>
#include <cstddef>
#include <deque>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr float       kGraphMinHz      = 65.0F;   // ~C2, below typical singing range
constexpr float       kGraphMaxHz      = 1200.0F; // ~D6, above typical singing range
constexpr std::size_t kAnalysisWindow  = 2048;    // samples fed to the detector each pass
constexpr std::size_t kHistoryCapacity = 400;     // graph columns (scrolling strip chart)

struct CliOptions {
    int  width      = 960;
    int  height     = 540;
    bool fullscreen = false;
};

CliOptions parseArgs(int argc, char** argv) {
    CliOptions opts;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (arg == "--width" && (i + 1) < argc) {
            opts.width = std::stoi(argv[++i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (arg == "--height" && (i + 1) < argc) {
            opts.height = std::stoi(argv[++i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (arg == "--fullscreen") {
            opts.fullscreen = true;
        }
    }
    return opts;
}

bool isQuitEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) { return true; }
    return event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE;
}

void pollEvents(bool& quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (isQuitEvent(event)) { quit = true; }
    }
}

// Appends newly captured samples and trims the buffer to the most recent
// analysis window (a simple fixed-size rolling window over live audio).
void updateRingBuffer(std::vector<float>& ring, ast::IAudioCapture& capture) {
    std::vector<float> newSamples;
    capture.readSamples(newSamples);
    ring.insert(ring.end(), newSamples.begin(), newSamples.end());
    if (ring.size() > kAnalysisWindow) {
        ring.erase(ring.begin(), ring.end() - static_cast<std::ptrdiff_t>(kAnalysisWindow));
    }
}

struct DisplayText {
    std::string freqText = "---";
    std::string noteText;
    ast::Colour colour{160, 160, 160, 255};
};

DisplayText buildDisplayText(const std::optional<ast::PitchResult>& result) {
    DisplayText text;
    if (!result) { return text; }

    const ast::Note note = ast::frequencyToNote(result->frequencyHz);
    text.colour   = ast::pitchClassColour(note.pitchClass);
    text.noteText = note.name;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << result->frequencyHz;
    text.freqText = oss.str() + " HZ";
    return text;
}

float yForFrequency(float freqHz, float graphTop, float graphBottom) {
    const float t = (std::log2(freqHz) - std::log2(kGraphMinHz))
                   / (std::log2(kGraphMaxHz) - std::log2(kGraphMinHz));
    return graphBottom - (t * (graphBottom - graphTop));
}

void drawGraphFrame(ast::IRenderer& renderer, float left, float top, float right, float bottom) {
    static const ast::Colour kFrameColour{60, 60, 70, 255};
    const std::vector<ast::Vec2> corners{
        {left, top}, {right, top}, {right, bottom}, {left, bottom}
    };
    renderer.drawLineStrip(corners, kFrameColour, true);
}

void drawHistory(ast::IRenderer& renderer,
                  const std::deque<std::optional<ast::PitchResult>>& history,
                  float graphLeft, float graphTop, float graphRight, float graphBottom) {
    ast::Vec2 prevPoint{};
    bool      havePrev = false;
    for (std::size_t i = 0; i < history.size(); ++i) {
        const float x = graphLeft
                       + (static_cast<float>(i) / static_cast<float>(kHistoryCapacity - 1))
                       * (graphRight - graphLeft);
        const std::optional<ast::PitchResult>& entry = history.at(i);
        if (!entry.has_value()) {
            havePrev = false;
            continue;
        }
        const float    y     = yForFrequency(entry->frequencyHz, graphTop, graphBottom);
        const ast::Vec2 point{x, y};
        if (havePrev) {
            const ast::Note note = ast::frequencyToNote(entry->frequencyHz);
            renderer.drawLine(prevPoint, point, ast::pitchClassColour(note.pitchClass));
        }
        prevPoint = point;
        havePrev  = true;
    }
}

void renderFrame(ast::IRenderer& renderer, const CliOptions& opts,
                  const std::optional<ast::PitchResult>& result,
                  const std::deque<std::optional<ast::PitchResult>>& history) {
    renderer.clear(ast::Colour{10, 10, 18, 255});

    const DisplayText text = buildDisplayText(result);
    ast::drawString(renderer, {30.0F, 24.0F}, 64.0F, text.freqText, text.colour);
    if (!text.noteText.empty()) {
        ast::drawString(renderer, {30.0F, 104.0F}, 44.0F, text.noteText, text.colour);
    }

    const float graphLeft   = 30.0F;
    const float graphRight  = static_cast<float>(opts.width) - 30.0F;
    const float graphTop    = 180.0F;
    const float graphBottom = static_cast<float>(opts.height) - 30.0F;
    drawGraphFrame(renderer, graphLeft, graphTop, graphRight, graphBottom);
    drawHistory(renderer, history, graphLeft, graphTop, graphRight, graphBottom);

    renderer.present();
}

} // namespace

int main(int argc, char* argv[]) {
    const CliOptions opts = parseArgs(argc, argv);

    ast::Platform platform("Singing Frequency Analyzer", opts.width, opts.height, opts.fullscreen);
    if (platform.window() == nullptr) {
        return 1;
    }

    ast::Sdl2Renderer renderer(platform.window());
    if (!renderer.isOk()) {
        return 1;
    }

    ast::Sdl2AudioCapture capture;
    if (!capture.isOk()) {
        std::cerr << "No microphone available — connect one and restart.\n";
    }

    const ast::PitchDetector detector(capture.sampleRate(), kGraphMinHz, kGraphMaxHz);

    std::vector<float>                          ring;
    std::deque<std::optional<ast::PitchResult>> history;

    bool quit = false;
    while (!quit) {
        pollEvents(quit);
        updateRingBuffer(ring, capture);

        std::optional<ast::PitchResult> result;
        if (ring.size() >= kAnalysisWindow) {
            result = detector.detect(ring);
        }

        history.push_back(result);
        while (history.size() > kHistoryCapacity) { history.pop_front(); }

        renderFrame(renderer, opts, result, history);
        SDL_Delay(16);
    }

    return 0;
}
