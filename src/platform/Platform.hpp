#pragma once
#include <memory>

struct SDL_Window; // forward declaration safe: SDL2 uses typedef struct SDL_Window SDL_Window

namespace ast {

struct SdlWindowDeleter {
    void operator()(SDL_Window* w) const;
};

class Platform {
public:
    Platform(const char* title, int width, int height);
    ~Platform();

    Platform(const Platform&)            = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&)                 = delete;
    Platform& operator=(Platform&&)      = delete;

    SDL_Window* window() const noexcept;

private:
    std::unique_ptr<SDL_Window, SdlWindowDeleter> window_;
};

} // namespace ast
