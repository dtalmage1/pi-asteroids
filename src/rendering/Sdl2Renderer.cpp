#include "rendering/Sdl2Renderer.hpp"
#include <SDL.h>

namespace ast {

void SdlRendererDeleter::operator()(SDL_Renderer* r) const {
    SDL_DestroyRenderer(r);
}

Sdl2Renderer::Sdl2Renderer(SDL_Window* window)
    : renderer_(SDL_CreateRenderer(window, -1, 0))
{}

Sdl2Renderer::~Sdl2Renderer() = default;

void Sdl2Renderer::clear(Colour background) {
    SDL_SetRenderDrawColor(renderer_.get(),
                           background.r, background.g, background.b, background.a);
    SDL_RenderClear(renderer_.get());
}

// Stub — drawing implemented in RND-1.
void Sdl2Renderer::drawLine(Vec2 /*a*/, Vec2 /*b*/, Colour /*c*/) {}

// Stub — drawing implemented in RND-1.
void Sdl2Renderer::drawLineStrip(const std::vector<Vec2>& /*points*/, Colour /*c*/, bool /*closed*/) {}

void Sdl2Renderer::present() {
    SDL_RenderPresent(renderer_.get());
}

Vec2 Sdl2Renderer::screenSize() const {
    int w = 0;
    int h = 0;
    SDL_GetRendererOutputSize(renderer_.get(), &w, &h);
    return {static_cast<float>(w), static_cast<float>(h)};
}

} // namespace ast
