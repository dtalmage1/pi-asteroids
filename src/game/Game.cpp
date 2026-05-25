#include "game/Game.hpp"

namespace ast {

Game::Game(IAudioSink& audio, Vec2 screenSize)
    : audio_(audio), screenSize_(screenSize) {}

void Game::update(float /*dt*/, const InputState& /*input*/) {}

void Game::render(IRenderer& /*renderer*/) const {}

GameState Game::state() const noexcept { return state_; }

} // namespace ast
