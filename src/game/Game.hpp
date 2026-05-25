#pragma once
#include "game/IAudioSink.hpp"
#include "game/IRenderer.hpp"
#include "game/InputState.hpp"
#include "game/Vec2.hpp"

namespace ast {

enum class GameState { Attract, Playing, PlayerDead, GameOver };

class Game {
public:
    Game(IAudioSink& audio, Vec2 screenSize);
    ~Game()                      = default;
    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&)                 = delete;
    Game& operator=(Game&&)      = delete;

    void      update(float dt, const InputState& input);
    void      render(IRenderer& renderer) const;
    GameState state() const noexcept;

private:
    IAudioSink& audio_;
    Vec2        screenSize_;
    GameState   state_ = GameState::Attract;
};

} // namespace ast
