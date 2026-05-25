#pragma once
#include "game/IAudioSink.hpp"
#include "game/IRenderer.hpp"
#include "game/InputState.hpp"
#include "game/Vec2.hpp"
#include "game/entities/Asteroid.hpp"
#include "game/entities/Projectile.hpp"
#include "game/entities/Ship.hpp"
#include <array>
#include <vector>

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
    const Ship& ship() const noexcept;

    void spawnAsteroid(Asteroid a);
    const std::vector<Asteroid>& asteroids() const noexcept;
    const std::array<Projectile, kMaxProjectiles>& projectiles() const noexcept;

private:
    IAudioSink&           audio_;
    Vec2                  screenSize_;
    GameState             state_ = GameState::Attract;
    Ship                  ship_;
    std::vector<Asteroid> asteroids_;
    std::array<Projectile, kMaxProjectiles> projectiles_{};
};

} // namespace ast
