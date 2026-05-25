#define SDL_MAIN_HANDLED  // prevent SDL from redefining main as SDL_main
#include "platform/Platform.hpp"
#include "rendering/Sdl2Renderer.hpp"
#include "input/Sdl2InputSource.hpp"
#include "audio/Sdl2AudioSink.hpp"
#include "game/Game.hpp"
#include "game/Colour.hpp"
#include <SDL.h>

int main() {
    ast::Platform platform("Asteroids", 800, 600);
    if (platform.window() == nullptr) {
        return 1;
    }

    ast::Sdl2Renderer    renderer(platform.window());
    ast::Sdl2InputSource inputSource;
    ast::Sdl2AudioSink   audioSink;
    ast::Game            game(audioSink, renderer.screenSize());

    static constexpr float kTargetDt = 1.0F / 60.0F;
    static constexpr float kMaxDt    = 0.05F;
    const Uint64           freq      = SDL_GetPerformanceFrequency();
    Uint64                 prevTime  = SDL_GetPerformanceCounter();

    bool quit = false;
    while (!quit) {
        const Uint64 now     = SDL_GetPerformanceCounter();
        const float  rawDt   = static_cast<float>(now - prevTime) / static_cast<float>(freq);
        const float  dt      = rawDt < kMaxDt ? rawDt : kMaxDt;
        prevTime = now;

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) { quit = true; }
        }

        const ast::InputState input = inputSource.query();
        game.update(dt, input);

        renderer.clear(ast::Colour{0, 0, 0, 255});
        game.render(renderer);
        renderer.present();

        const float elapsed = static_cast<float>(SDL_GetPerformanceCounter() - now)
                            / static_cast<float>(freq);
        if (elapsed < kTargetDt) {
            SDL_Delay(static_cast<Uint32>((kTargetDt - elapsed) * 1000.0F));
        }
    }

    return 0;
}
