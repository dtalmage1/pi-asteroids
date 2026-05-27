#define SDL_MAIN_HANDLED  // prevent SDL from redefining main as SDL_main
#include "platform/Platform.hpp"
#include "rendering/Sdl2Renderer.hpp"
#include "input/Sdl2InputSource.hpp"
#include "audio/Sdl2AudioSink.hpp"
#include "game/Game.hpp"
#include "game/Colour.hpp"
#include <SDL.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

int main(int argc, char* argv[]) {
    int  winWidth   = 800;
    int  winHeight  = 600;
    bool fullscreen = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (arg == "--width"  && (i + 1) < argc) {
            winWidth  = std::stoi(argv[++i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (arg == "--height" && (i + 1) < argc) {
            winHeight = std::stoi(argv[++i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else if (arg == "--fullscreen") {
            fullscreen = true;
        }
    }

    ast::Platform platform("Asteroids", winWidth, winHeight, fullscreen);
    if (platform.window() == nullptr) {
        return 1;
    }

#ifdef ASTEROIDS_VSYNC
    static constexpr bool kVsync = true;
#else
    static constexpr bool kVsync = false;
#endif
    ast::Sdl2Renderer renderer(platform.window(), kVsync);
    if (!renderer.isOk()) {
        return 1;
    }
    ast::Sdl2InputSource inputSource;
    ast::Sdl2AudioSink   audioSink;
    ast::Game            game(audioSink, renderer.screenSize());

    static constexpr float kTargetDt = 1.0F / 60.0F;
    static constexpr float kMaxDt    = 0.05F;
    const Uint64           freq      = SDL_GetPerformanceFrequency();
    Uint64                 prevTime  = SDL_GetPerformanceCounter();

#ifndef NDEBUG
    float fpsAccumTime   = 0.0F;
    int   fpsAccumFrames = 0;
#endif

    bool quit = false;
    while (!quit) {
        const Uint64 now     = SDL_GetPerformanceCounter();
        const float  rawDt   = static_cast<float>(now - prevTime) / static_cast<float>(freq);
        const float  dt      = rawDt < kMaxDt ? rawDt : kMaxDt;
        prevTime = now;

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) { quit = true; }
            inputSource.handleEvent(event);
        }

        const ast::InputState input = inputSource.query();
        game.update(dt, input);

        renderer.clear(ast::Colour{0, 0, 0, 255});
        game.render(renderer);
        renderer.present();

#ifndef NDEBUG
        fpsAccumTime   += dt;
        fpsAccumFrames += 1;
        if (fpsAccumTime >= 2.0F) {
            std::cerr << "FPS: "
                      << std::fixed << std::setprecision(1)
                      << (static_cast<float>(fpsAccumFrames) / fpsAccumTime)
                      << '\n';
            fpsAccumTime   = 0.0F;
            fpsAccumFrames = 0;
        }
#endif

        if constexpr (!kVsync) {
            const float elapsed = static_cast<float>(SDL_GetPerformanceCounter() - now)
                                / static_cast<float>(freq);
            if (elapsed < kTargetDt) {
                SDL_Delay(static_cast<Uint32>((kTargetDt - elapsed) * 1000.0F));
            }
        }
    }

    return 0;
}
