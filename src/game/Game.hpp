#pragma once
#include "game/IAudioSink.hpp"
#include "game/IRenderer.hpp"
#include "game/InputState.hpp"
#include "game/ScoreTable.hpp"
#include "game/Vec2.hpp"
#include "game/entities/Asteroid.hpp"
#include "game/entities/Particle.hpp"
#include "game/entities/Projectile.hpp"
#include "game/entities/Saucer.hpp"
#include "game/entities/Ship.hpp"
#include <array>
#include <cstdint>
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
    int       score() const noexcept;
    int       waveNumber() const noexcept;
    int       lives() const noexcept;

    void spawnAsteroid(Asteroid a);
    void spawnProjectile(Projectile p);
    void activateSaucer(SaucerSize size, Vec2 pos);
    const std::vector<Asteroid>& asteroids() const noexcept;
    const std::array<Projectile, kMaxProjectiles>& projectiles() const noexcept;
    const std::array<Particle,   kMaxParticles>&   particles()   const noexcept;
    const ScoreTable& scoreTable() const noexcept;
    const Saucer&     saucer()     const noexcept;

private:
    void tryFire(const InputState& input);
    void tryHyperspace(const InputState& input);
    void renderShip(IRenderer& renderer) const;
    void killShip();
    void spawnSaucer();
    void fireSaucerProjectile();
    void tickSaucer(float dt);
    void checkCollisions();
    void checkSaucerCollisions();
    void checkShipCollisions();
    void checkSaucerVsShipCollisions();
    void spawnExplosion(Vec2 pos, int count);
    void tickParticles(float dt);
    void tickWave(float dt);
    void tickBeat(float dt);
    void startWave();
    void tickRespawn(float dt);
    void tickGameOver(float dt);

    IAudioSink&           audio_;
    Vec2                  screenSize_;
    float                 scale_;
    GameState             state_          = GameState::Attract;
    ScoreTable            scoreTable_;
    Ship                  ship_;
    std::vector<Asteroid> asteroids_;
    std::array<Projectile, kMaxProjectiles> projectiles_{};
    std::array<Particle,   kMaxParticles>   particles_{};
    std::uint32_t         splitSeed_        = 0;
    std::uint32_t         particleSeed_     = 0U;
    std::uint32_t         hyperspaceSeed_   = 0U;
    Saucer                saucer_           {};
    float                 saucerSpawnTimer_ = 0.0F;
    std::uint32_t         saucerSeed_       = 0U;
    std::uint32_t         saucerFireSeed_   = 0U;
    int                   score_          = 0;
    int                   waveNumber_     = 0;
    float                 interWaveTimer_ = 0.0F;
    bool                  prevAllClear_   = true;
    std::uint32_t         waveSeed_       = 1000U;
    int                   lives_              = 0;
    float                 respawnTimer_       = 0.0F;
    int                   nextExtraLifeScore_ = 0;
    float                 gameOverTimer_      = 0.0F;
    float                 beatTimer_          = 0.0F;
    bool                  beatPhase_          = false;
    bool                  prevFireInput_      = false;
    bool                  prevHyperspaceInput_= false;
    std::uint32_t         frameCount_         = 0U;
};

} // namespace ast
