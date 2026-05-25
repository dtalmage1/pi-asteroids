#define SDL_MAIN_HANDLED  // prevent SDL from redefining main as SDL_main
#include "platform/Platform.hpp"
#include "rendering/Sdl2Renderer.hpp"
#include "game/Colour.hpp"
#include <SDL.h>

int main() {
    ast::Platform platform("Asteroids", 800, 600);
    if (platform.window() == nullptr) {
        return 1;
    }

    ast::Sdl2Renderer renderer(platform.window());

    static constexpr float kTargetDt = 1.0F / 60.0F;
    const Uint64 freq = SDL_GetPerformanceFrequency();

    bool quit = false;
    while (!quit) {
        const Uint64 frameStart = SDL_GetPerformanceCounter();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) { quit = true; }
        }

        renderer.clear(ast::Colour{0, 0, 0, 255});
        renderer.present();

        const Uint64 frameEnd  = SDL_GetPerformanceCounter();
        const float  frameTime = static_cast<float>(frameEnd - frameStart)
                               / static_cast<float>(freq);
        if (frameTime < kTargetDt) {
            SDL_Delay(static_cast<Uint32>((kTargetDt - frameTime) * 1000.0F));
        }
    }

    return 0;
}
