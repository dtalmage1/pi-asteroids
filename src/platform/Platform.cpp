#include "platform/Platform.hpp"
#include <SDL.h>
#include <iostream>

namespace ast {

void SdlWindowDeleter::operator()(SDL_Window* w) const {
    SDL_DestroyWindow(w);
}

Platform::Platform(const char* title, int width, int height, bool fullscreen) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return;
    }
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL audio init failed: " << SDL_GetError() << " (audio disabled)\n";
    }
    const Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0U;
    window_.reset(SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags));
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    }
}

Platform::~Platform() {
    window_.reset(); // destroy window before SDL_Quit
    SDL_Quit();
}

SDL_Window* Platform::window() const noexcept {
    return window_.get();
}

} // namespace ast
