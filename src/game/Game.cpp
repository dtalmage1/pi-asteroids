#include "game/Game.hpp"
#include "game/physics/Physics.hpp"

namespace {
constexpr float kShipDrag = 0.5F;
} // namespace

namespace ast {

Game::Game(IAudioSink& audio, Vec2 screenSize)
    : audio_(audio), screenSize_(screenSize) {}

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

void Game::render(IRenderer& /*renderer*/) const {}

GameState Game::state() const noexcept { return state_; }

const Ship& Game::ship() const noexcept { return ship_; }

} // namespace ast
