#pragma once
#include <memory>
#include <vector>
#include "game/IRenderer.hpp"

// Forward-declare SDL types so consumers of this header don't need SDL.h.
struct SDL_Window;   // NOLINT(modernize-use-using) — C forward declaration of C struct
struct SDL_Renderer; // NOLINT(modernize-use-using)

namespace ast {

struct SdlRendererDeleter {
    void operator()(SDL_Renderer* r) const;
};

class Sdl2Renderer : public IRenderer {
public:
    explicit Sdl2Renderer(SDL_Window* window, bool vsync = false);
    ~Sdl2Renderer() override;

    Sdl2Renderer(const Sdl2Renderer&)            = delete;
    Sdl2Renderer& operator=(const Sdl2Renderer&) = delete;
    Sdl2Renderer(Sdl2Renderer&&)                 = delete;
    Sdl2Renderer& operator=(Sdl2Renderer&&)      = delete;

    bool isOk() const noexcept;

    void clear(Colour background) override;
    void drawLine(Vec2 a, Vec2 b, Colour c) override;
    void drawLineStrip(const std::vector<Vec2>& points, Colour c, bool closed) override;
    void present() override;
    Vec2 screenSize() const override;

private:
    std::unique_ptr<SDL_Renderer, SdlRendererDeleter> renderer_;
};

} // namespace ast
