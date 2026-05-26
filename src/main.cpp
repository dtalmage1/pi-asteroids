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

int main() {
    ast::Platform platform("Asteroids", 800, 600);
    if (platform.window() == nullptr) {
        return 1;
    }

#ifdef ASTEROIDS_VSYNC
    static constexpr bool kVsync = true;
#else
    static constexpr bool kVsync = false;
#endif
    ast::Sdl2Renderer    renderer(platform.window(), kVsync);
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
