#include "rendering/Sdl2Renderer.hpp"
#include <SDL.h>
#include <iostream>
#include <iterator>

namespace ast {

void SdlRendererDeleter::operator()(SDL_Renderer* r) const {
    SDL_DestroyRenderer(r);
}

Sdl2Renderer::Sdl2Renderer(SDL_Window* window, bool vsync)
    : renderer_(SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : 0))
{
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
    }
}

Sdl2Renderer::~Sdl2Renderer() = default;

bool Sdl2Renderer::isOk() const noexcept {
    return renderer_ != nullptr;
}

void Sdl2Renderer::clear(Colour background) {
    if (!renderer_) { return; }
    SDL_SetRenderDrawColor(renderer_.get(),
                           background.r, background.g, background.b, background.a);
    SDL_RenderClear(renderer_.get());
}

void Sdl2Renderer::drawLine(Vec2 a, Vec2 b, Colour c) {
    if (!renderer_) { return; }
    SDL_SetRenderDrawColor(renderer_.get(), c.r, c.g, c.b, c.a);
    SDL_RenderDrawLineF(renderer_.get(), a.x, a.y, b.x, b.y);
}

void Sdl2Renderer::drawLineStrip(const std::vector<Vec2>& points, Colour c, bool closed) {
    if (!renderer_ || points.size() < 2) { return; }
    SDL_SetRenderDrawColor(renderer_.get(), c.r, c.g, c.b, c.a);
    for (auto it = points.cbegin(); std::next(it) != points.cend(); ++it) {
        const auto nextIt = std::next(it);
        SDL_RenderDrawLineF(renderer_.get(), it->x, it->y, nextIt->x, nextIt->y);
    }
    if (closed) {
        SDL_RenderDrawLineF(renderer_.get(),
                            points.back().x,  points.back().y,
                            points.front().x, points.front().y);
    }
}

void Sdl2Renderer::present() {
    if (!renderer_) { return; }
    SDL_RenderPresent(renderer_.get());
}

Vec2 Sdl2Renderer::screenSize() const {
    if (!renderer_) { return {0.0F, 0.0F}; }
    int w = 0;
    int h = 0;
    SDL_GetRendererOutputSize(renderer_.get(), &w, &h);
    return {static_cast<float>(w), static_cast<float>(h)};
}

} // namespace ast
