#include "game/Game.hpp"
#include "game/Colour.hpp"
#include "game/physics/Physics.hpp"
#include <array>
#include <cmath>
#include <vector>

namespace {
constexpr float kShipDrag = 0.5F;
} // namespace

namespace ast {

namespace {

// Local-space chevron vertices (angle=0 = nose pointing up, Y-down screen).
// Order: nose, right wing, tail notch, left wing — closed strip forms the hull.
constexpr std::array<ast::Vec2, 4> kShipShape{{
    { 0.0F, -15.0F},  // nose
    { 9.0F,   9.0F},  // right wing
    { 0.0F,   4.0F},  // tail notch
    {-9.0F,   9.0F},  // left wing
}};

std::vector<ast::Vec2> buildShipVertices(ast::Vec2 pos, float angle) {
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    std::vector<ast::Vec2> verts;
    verts.reserve(kShipShape.size());
    for (const auto& v : kShipShape) {
        verts.push_back({
            pos.x + v.x * cosA - v.y * sinA,
            pos.y + v.x * sinA + v.y * cosA
        });
    }
    return verts;
}

} // namespace

Game::Game(IAudioSink& audio, Vec2 screenSize)
    : audio_(audio), screenSize_(screenSize)
{
    ship_.position = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
}

void Game::update(float dt, const InputState& /*input*/) {
    if (state_ == GameState::Playing && ship_.active) {
        ship_.velocity = applyDrag(ship_.velocity, kShipDrag, dt);
        ship_.position = integratePosition(ship_.position, ship_.velocity, dt);
        ship_.position = wrapPosition(ship_.position, screenSize_);
        if (ship_.invincTimer > 0.0F) {
            ship_.invincTimer -= dt;
        }
    }
}

void Game::render(IRenderer& renderer) const {
    if (ship_.active) {
        renderer.drawLineStrip(
            buildShipVertices(ship_.position, ship_.angle),
            Colour{255, 255, 255, 255},
            true);
    }
}

GameState Game::state() const noexcept { return state_; }

const Ship& Game::ship() const noexcept { return ship_; }

} // namespace ast
