#include "game/Game.hpp"
#include "game/Colour.hpp"
#include "game/Glyph.hpp"
#include "game/Wave.hpp"
#include "game/physics/Collision.hpp"
#include "game/physics/Physics.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace {
constexpr float kShipDrag              = 0.5F;
constexpr float kRotationRate          = 3.5F;    // rad/s (~200 deg/s)
constexpr float kThrustAccel           = 200.0F;  // pixels/s²
constexpr float kProjectileSpeed       = 500.0F;  // pixels/s
constexpr float kProjectileLifetime    = 0.75F;   // seconds
constexpr float kShipNoseOffset        = 15.0F;   // pixels from ship centre to nose tip
constexpr float kProjectileHalfLen     =  2.0F;   // half visual length of projectile line
constexpr float kProjectileRadius      =  2.0F;   // collision radius (matches visual size)
constexpr float kSplitAngle            =  0.5236F; // π/6 ≈ 30° divergence per child
constexpr float kSplitSpeedMult        =  1.3F;    // children are 30% faster than parent
constexpr float kSplitMinSpeed         = 60.0F;    // floor speed for stationary parent (pixels/s)
constexpr float kSplitAngularMult      =  1.5F;    // children spin faster than parent
constexpr int   kScoreLarge            =  20;
constexpr int   kScoreMedium           =  50;
constexpr int   kScoreSmall            = 100;
constexpr float kInterWaveDelay        =  2.0F;   // seconds between wave clear and next spawn
constexpr float kPi                    =  3.14159265358979323846F;
constexpr float kAsteroidMinSpeed      = 50.0F;   // pixels/s
constexpr float kAsteroidMaxSpeed      = 120.0F;  // pixels/s
constexpr float kAsteroidAngularVelMax =  1.5F;   // rad/s
constexpr int   kInitialLives          =  3;
constexpr float kRespawnDelay          =  3.0F;   // seconds before ship respawns
constexpr float kInvincDuration        =  3.0F;   // seconds of invincibility after respawn
constexpr int   kExtraLifeScore        = 10000;
constexpr float kFlashPeriod           =  0.1F;   // half-period of ship flash while invincible
constexpr float kGameOverDelay         =  5.0F;   // seconds before auto-returning to Attract
constexpr float kParticleLifetimeMin   =  0.4F;   // seconds
constexpr float kParticleLifetimeMax   =  1.0F;   // seconds
constexpr float kParticleSpeedMin      = 60.0F;   // pixels/s
constexpr float kParticleSpeedMax      = 180.0F;  // pixels/s
constexpr float kParticleHalfLen       =  3.0F;   // half visual length of particle line
constexpr int   kAsteroidParticleCount  =  8;
constexpr int   kShipParticleCount      = 12;
constexpr std::uint32_t kHyperspaceDeathOdds  = 5U;    // 1-in-5 chance of destruction on arrival
constexpr float kSaucerSpawnInterval          = 15.0F;  // seconds between saucer appearances
constexpr float kLargeSaucerSpeed             = 80.0F;  // pixels/s
constexpr float kSaucerFireInterval           =  1.5F;  // seconds between saucer shots
constexpr float kSaucerProjectileSpeed        = 300.0F; // pixels/s
constexpr int   kScoreSaucerLarge             = 200;
constexpr int   kScoreSaucerSmall             = 1000;
constexpr float kSmallSaucerSpreadMax         = 1.0471796F;  // π/3
constexpr float kSmallSaucerSpreadStep        = 0.2617994F;  // π/12 per wave
constexpr float kAttractTitleCharHeightFrac  = 0.10F;
constexpr float kAttractTitleYFrac           = 0.30F;
constexpr float kAttractPromptCharHeightFrac = 0.05F;
constexpr float kAttractPromptYFrac          = 0.55F;
constexpr float kAttractScoreCharHeightFrac  = 0.04F;
constexpr float kAttractScoreStartYFrac      = 0.65F;
constexpr float kAttractScoreLineHeightFrac  = 0.06F;
constexpr float kHudMargin             = 10.0F;   // pixels from screen edge to HUD elements
constexpr float kHudCharHeightFrac     =  0.04F;  // score digit height as fraction of screen height
constexpr float kShipIconSpan          = 24.0F;   // kShipShape vertical extent: nose(-15) to tail(+9)
constexpr float kIconNoseFrac          =  0.625F; // 15/24: nose-to-centre as fraction of icon span
constexpr float kIconHalfWidthFrac     =  0.375F; // 9/24: half-width of icon as fraction of icon span
} // namespace

namespace ast {

namespace {

// Saucer shape in normalised space (radius=1). Body is a closed 6-point strip;
// dome is an open 4-point strip sitting on top of the body.
constexpr std::array<ast::Vec2, 6> kSaucerBody{{
    {-1.0F,  0.0F},
    {-0.5F, -0.4F},
    { 0.5F, -0.4F},
    { 1.0F,  0.0F},
    { 0.5F,  0.4F},
    {-0.5F,  0.4F},
}};
constexpr std::array<ast::Vec2, 4> kSaucerDome{{
    {-0.5F, -0.4F},
    {-0.3F, -0.8F},
    { 0.3F, -0.8F},
    { 0.5F, -0.4F},
}};

// Scale a shape array by radius and translate to world position.
template <std::size_t N>
std::vector<ast::Vec2> scaledVerts(
    const std::array<ast::Vec2, N>& src, ast::Vec2 pos, float r)
{
    std::vector<ast::Vec2> v;
    v.reserve(N);
    for (const auto& s : src) {
        v.push_back({pos.x + (s.x * r), pos.y + (s.y * r)});
    }
    return v;
}

// Local-space chevron vertices (angle=0 = nose pointing up, Y-down screen).
// Order: nose, right wing, tail notch, left wing — closed strip forms the hull.
constexpr std::array<ast::Vec2, 4> kShipShape{{
    { 0.0F, -15.0F},  // nose
    { 9.0F,   9.0F},  // right wing
    { 0.0F,   4.0F},  // tail notch
    {-9.0F,   9.0F},  // left wing
}};

std::vector<ast::Vec2> buildShipVertices(ast::Vec2 pos, float angle, float scale = 1.0F) {
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    std::vector<ast::Vec2> verts;
    verts.reserve(kShipShape.size());
    for (const auto& v : kShipShape) {
        const float sx = v.x * scale;
        const float sy = v.y * scale;
        verts.push_back({
            pos.x + (sx * cosA) - (sy * sinA),
            pos.y + (sx * sinA) + (sy * cosA)
        });
    }
    return verts;
}

int asteroidScore(ast::AsteroidSize size) noexcept {
    if (size == ast::AsteroidSize::Large)  { return kScoreLarge;  }
    if (size == ast::AsteroidSize::Medium) { return kScoreMedium; }
    return kScoreSmall;
}

// Returns the beat interval (seconds) for the given number of active asteroids.
// Interval shrinks linearly as rocks are destroyed, creating tension.
float beatInterval(int count) noexcept {
    constexpr float kMax = 0.9F;
    constexpr float kMin = 0.25F;
    constexpr int   kRef = 28;  // 4 large fully split = max visible in wave 1
    const float t = std::min(1.0F, static_cast<float>(count) / static_cast<float>(kRef));
    return kMin + (t * (kMax - kMin));
}

ast::SoundId explosionSound(ast::AsteroidSize size) noexcept {
    if (size == ast::AsteroidSize::Large)  { return ast::SoundId::ExplosionLarge;  }
    if (size == ast::AsteroidSize::Medium) { return ast::SoundId::ExplosionMedium; }
    return ast::SoundId::ExplosionSmall;
}

std::vector<ast::Asteroid> spawnChildren(const ast::Asteroid& parent, uint32_t seed, float scale) {
    if (parent.size == ast::AsteroidSize::Small) { return {}; }
    const ast::AsteroidSize childSize = (parent.size == ast::AsteroidSize::Large)
                                        ? ast::AsteroidSize::Medium
                                        : ast::AsteroidSize::Small;
    const float parentSpeed = parent.velocity.length();
    const float childSpeed  = std::max((parentSpeed * kSplitSpeedMult), kSplitMinSpeed * scale);
    const ast::Vec2 dir = (parentSpeed > 0.0F)
                          ? parent.velocity.normalised()
                          : ast::Vec2{0.0F, -1.0F};
    const float cosA = std::cos(kSplitAngle);
    const float sinA = std::sin(kSplitAngle);
    const ast::Vec2 vel1 = ast::Vec2{(dir.x * cosA) - (dir.y * sinA),
                                      (dir.x * sinA) + (dir.y * cosA)} * childSpeed;
    const ast::Vec2 vel2 = ast::Vec2{(dir.x * cosA) + (dir.y * sinA),
                                     -(dir.x * sinA) + (dir.y * cosA)} * childSpeed;
    std::vector<ast::Asteroid> children;
    children.reserve(2);
    ast::Asteroid c0;
    c0.position   = parent.position;
    c0.velocity   = vel1;
    c0.angularVel = parent.angularVel * kSplitAngularMult;
    c0.size       = childSize;
    c0.shape      = ast::generateAsteroidShape(childSize, 10, seed, scale);
    c0.active     = true;
    children.push_back(std::move(c0));
    ast::Asteroid c1;
    c1.position   = parent.position;
    c1.velocity   = vel2;
    c1.angularVel = parent.angularVel * kSplitAngularMult;
    c1.size       = childSize;
    c1.shape      = ast::generateAsteroidShape(childSize, 10, seed + 1U, scale);
    c1.active     = true;
    children.push_back(std::move(c1));
    return children;
}

std::vector<ast::Vec2> buildAsteroidVertices(const ast::Asteroid& rock) {
    const float cosA = std::cos(rock.angle);
    const float sinA = std::sin(rock.angle);
    std::vector<ast::Vec2> verts;
    verts.reserve(rock.shape.size());
    for (const auto& v : rock.shape) {
        verts.push_back({
            rock.position.x + (v.x * cosA) - (v.y * sinA),
            rock.position.y + (v.x * sinA) + (v.y * cosA)
        });
    }
    return verts;
}

} // namespace

Game::Game(IAudioSink& audio, Vec2 screenSize)
    : audio_(audio), screenSize_(screenSize),
      scale_(screenSize.y / 600.0F), nextExtraLifeScore_(kExtraLifeScore)
{
    ship_.position = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
}

void Game::update(float dt, const InputState& input) {
    if (state_ == GameState::Attract && input.start) {
        state_          = GameState::Playing;
        ship_.position  = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
        ship_.velocity  = {0.0F, 0.0F};
        ship_.angle     = 0.0F;
        ship_.active    = true;
        asteroids_.clear();
        projectiles_    = {};
        score_              = 0;
        splitSeed_          = 0;
        particleSeed_       = 0U;
        hyperspaceSeed_     = 0U;
        saucer_             = {};
        saucerSpawnTimer_   = kSaucerSpawnInterval;
        saucerSeed_         = 0U;
        saucerFireSeed_     = 0U;
        waveNumber_         = 0;
        interWaveTimer_     = kInterWaveDelay;
        prevAllClear_       = true;
        waveSeed_           = 1000U;
        beatTimer_          = 0.0F;
        beatPhase_          = false;
        lives_              = kInitialLives;
        respawnTimer_       = 0.0F;
        particles_          = {};
        nextExtraLifeScore_ = kExtraLifeScore;
    }

    if (state_ == GameState::GameOver && input.start) {
        scoreTable_.submit(score_);
        state_ = GameState::Attract;
    }

    if (state_ == GameState::Playing && ship_.active) {
        if (input.rotateLeft)  { ship_.angle -= kRotationRate * dt; }
        if (input.rotateRight) { ship_.angle += kRotationRate * dt; }
        ship_.angle = wrapAngle(ship_.angle);

        ship_.thrusting = input.thrust;
        if (input.thrust) {
            ship_.velocity = applyThrust(ship_.velocity, ship_.angle, kThrustAccel * scale_, dt);
            audio_.loop(SoundId::Thrust);
        } else {
            audio_.stop(SoundId::Thrust);
        }

        ship_.velocity = applyDrag(ship_.velocity, kShipDrag, dt);
        ship_.position = integratePosition(ship_.position, ship_.velocity, dt);
        ship_.position = wrapPosition(ship_.position, screenSize_);

        if (ship_.invincTimer > 0.0F) { ship_.invincTimer -= dt; }

        tryFire(input);
        tryHyperspace(input);
    }

    for (auto& p : projectiles_) {
        if (!p.active) { continue; }
        p.lifetime -= dt;
        if (p.lifetime <= 0.0F) {
            p.active = false;
            continue;
        }
        p.position = integratePosition(p.position, p.velocity, dt);
        p.position = wrapPosition(p.position, screenSize_);
    }

    for (auto& rock : asteroids_) {
        if (!rock.active) { continue; }
        rock.angle    = wrapAngle(rock.angle + (rock.angularVel * dt));
        rock.position = integratePosition(rock.position, rock.velocity, dt);
        rock.position = wrapPosition(rock.position, screenSize_);
    }

    checkCollisions();
    checkSaucerCollisions();
    checkShipCollisions();
    checkSaucerVsShipCollisions();
    tickParticles(dt);
    tickSaucer(dt);
    tickWave(dt);
    tickBeat(dt);
    tickRespawn(dt);
    tickGameOver(dt);
}

void Game::tryFire(const InputState& input) {
    if (!input.fire) { return; }
    Projectile* slot = nullptr;
    for (auto& candidate : projectiles_) {
        if (!candidate.active) { slot = &candidate; break; }
    }
    if (slot != nullptr) {
        const Vec2 nose{std::sin(ship_.angle), -std::cos(ship_.angle)};
        slot->position = ship_.position + (nose * (kShipNoseOffset * scale_));
        slot->velocity = ship_.velocity + (nose * (kProjectileSpeed * scale_));
        slot->lifetime = kProjectileLifetime;
        slot->owner    = ProjectileOwner::Player;
        slot->active   = true;
        audio_.play(SoundId::Fire);
    }
}

void Game::tryHyperspace(const InputState& input) {
    if (!input.hyperspace) { return; }
    ++hyperspaceSeed_;
    std::minstd_rand rng(hyperspaceSeed_);
    const bool dies = ((rng() % kHyperspaceDeathOdds) == 0U);
    const float nx = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
    const float ny = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
    ship_.position = {nx * screenSize_.x, ny * screenSize_.y};
    if (dies) {
        killShip();
    }
}

void Game::spawnSaucer() {
    ++saucerSeed_;
    std::minstd_rand rng(saucerSeed_);
    const bool fromRight = ((rng() % 2U) == 0U);
    const float yFrac    = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
    const float y        = (screenSize_.y * 0.1F) + (yFrac * (screenSize_.y * 0.8F));
    const bool isSmall   = (waveNumber_ >= 2) && ((rng() % 2U) == 0U);
    saucer_.size      = isSmall ? SaucerSize::Small : SaucerSize::Large;
    const float r        = Saucer::radius(saucer_.size) * scale_;
    saucer_.active    = true;
    saucer_.fireTimer = kSaucerFireInterval;
    if (fromRight) {
        saucer_.position = {screenSize_.x + r, y};
        saucer_.velocity = {-(kLargeSaucerSpeed * scale_), 0.0F};
    } else {
        saucer_.position = {-r, y};
        saucer_.velocity = {kLargeSaucerSpeed * scale_, 0.0F};
    }
}

void Game::fireSaucerProjectile() {
    Projectile* slot = nullptr;
    for (auto& candidate : projectiles_) {
        if (!candidate.active) { slot = &candidate; break; }
    }
    if (slot == nullptr) { return; }
    ++saucerFireSeed_;
    std::minstd_rand rng(saucerFireSeed_);
    float angle = 0.0F;
    if (saucer_.size == SaucerSize::Small && ship_.active) {
        const float dx       = ship_.position.x - saucer_.position.x;
        const float dy       = ship_.position.y - saucer_.position.y;
        const float aimAngle = std::atan2(dx, -dy);
        const float spread   = std::max(0.0F, kSmallSaucerSpreadMax -
                                        (static_cast<float>(waveNumber_ - 1) * kSmallSaucerSpreadStep));
        const float frac     = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
        angle = aimAngle + ((frac * (2.0F * spread)) - spread);
    } else {
        const float frac = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
        angle = frac * (2.0F * kPi);
    }
    const Vec2  dir{std::sin(angle), -std::cos(angle)};
    slot->position = saucer_.position;
    slot->velocity = dir * (kSaucerProjectileSpeed * scale_);
    slot->lifetime = kProjectileLifetime;
    slot->owner    = ProjectileOwner::Saucer;
    slot->active   = true;
    audio_.play(SoundId::SaucerFire);
}

void Game::tickSaucer(float dt) {
    if (state_ == GameState::Attract || state_ == GameState::GameOver) { return; }

    if (saucer_.active) {
        saucer_.position.x += saucer_.velocity.x * dt;
        saucer_.position.y += saucer_.velocity.y * dt;
        saucer_.fireTimer -= dt;
        if (saucer_.fireTimer <= 0.0F) {
            saucer_.fireTimer = kSaucerFireInterval;
            fireSaucerProjectile();
        }
        const SoundId engineSound = (saucer_.size == SaucerSize::Small)
                                        ? SoundId::SaucerEngineSmall
                                        : SoundId::SaucerEngine;
        audio_.loop(engineSound);
        const float r = Saucer::radius(saucer_.size) * scale_;
        if ((saucer_.position.x < (-r)) || (saucer_.position.x > (screenSize_.x + r))) {
            saucer_.active    = false;
            saucerSpawnTimer_ = kSaucerSpawnInterval;
            audio_.stop(SoundId::SaucerEngine);
            audio_.stop(SoundId::SaucerEngineSmall);
        }
    } else {
        saucerSpawnTimer_ -= dt;
        if (saucerSpawnTimer_ <= 0.0F) {
            spawnSaucer();
        }
    }
}

void Game::checkCollisions() {
    std::vector<Asteroid> spawned;
    for (auto& p : projectiles_) {
        if (!p.active || p.owner != ProjectileOwner::Player) { continue; }
        for (auto& rock : asteroids_) {
            if (!rock.active) { continue; }
            if (circlesOverlap(p.position, kProjectileRadius * scale_,
                               rock.position, Asteroid::radius(rock.size) * scale_)) {
                p.active      = false;
                spawnExplosion(rock.position, kAsteroidParticleCount);
                score_       += asteroidScore(rock.size);
                if (score_ >= nextExtraLifeScore_) {
                    nextExtraLifeScore_ += kExtraLifeScore;
                    ++lives_;
                }
                auto children = spawnChildren(rock, splitSeed_, scale_);
                splitSeed_   += 2U;
                for (auto& child : children) { spawned.push_back(std::move(child)); }
                rock.active   = false;
                audio_.play(explosionSound(rock.size));
                break;
            }
        }
    }
    for (auto& a : spawned) { asteroids_.push_back(std::move(a)); }
}

void Game::checkSaucerCollisions() {
    if (!saucer_.active) { return; }
    for (auto& p : projectiles_) {
        if (!p.active || p.owner != ProjectileOwner::Player) { continue; }
        if (circlesOverlap(p.position, kProjectileRadius * scale_,
                           saucer_.position, Saucer::radius(saucer_.size) * scale_)) {
            p.active          = false;
            saucer_.active    = false;
            saucerSpawnTimer_ = kSaucerSpawnInterval;
            spawnExplosion(saucer_.position, kAsteroidParticleCount);
            score_           += (saucer_.size == SaucerSize::Small) ? kScoreSaucerSmall : kScoreSaucerLarge;
            if (score_ >= nextExtraLifeScore_) {
                nextExtraLifeScore_ += kExtraLifeScore;
                ++lives_;
            }
            audio_.stop(SoundId::SaucerEngine);
            audio_.stop(SoundId::SaucerEngineSmall);
            audio_.play(SoundId::ExplosionLarge);
            return;
        }
    }
}

void Game::checkShipCollisions() {
    if (state_ != GameState::Playing) { return; }
    if (!ship_.active || ship_.invincTimer > 0.0F) { return; }
    for (const auto& rock : asteroids_) {
        if (!rock.active) { continue; }
        if (circlesOverlap(ship_.position, Ship::kRadius * scale_,
                           rock.position, Asteroid::radius(rock.size) * scale_)) {
            killShip();
            return;
        }
    }
}

void Game::spawnAsteroid(Asteroid a) {
    asteroids_.push_back(std::move(a));
}

void Game::spawnProjectile(Projectile p) {
    for (auto& slot : projectiles_) {
        if (!slot.active) { slot = p; return; }
    }
}

void Game::activateSaucer(SaucerSize size, Vec2 pos) {
    saucer_.size      = size;
    saucer_.position  = pos;
    saucer_.velocity  = {0.0F, 0.0F};
    saucer_.fireTimer = kSaucerFireInterval;
    saucer_.active    = true;
}

void Game::killShip() {
    ship_.active  = false;
    spawnExplosion(ship_.position, kShipParticleCount);
    --lives_;
    respawnTimer_ = kRespawnDelay;
    state_        = GameState::PlayerDead;
    audio_.stop(SoundId::Thrust);
    audio_.play(SoundId::ExplosionLarge);
}

void Game::checkSaucerVsShipCollisions() {
    if (state_ != GameState::Playing) { return; }
    if (!ship_.active || ship_.invincTimer > 0.0F) { return; }
    for (auto& p : projectiles_) {
        if (!p.active || p.owner != ProjectileOwner::Saucer) { continue; }
        if (circlesOverlap(p.position, kProjectileRadius * scale_,
                           ship_.position, Ship::kRadius * scale_)) {
            p.active = false;
            killShip();
            return;
        }
    }
    if (saucer_.active) {
        if (circlesOverlap(saucer_.position, Saucer::radius(saucer_.size) * scale_,
                           ship_.position, Ship::kRadius * scale_)) {
            killShip();
        }
    }
}

const std::vector<Asteroid>& Game::asteroids() const noexcept {
    return asteroids_;
}

const std::array<Projectile, kMaxProjectiles>& Game::projectiles() const noexcept {
    return projectiles_;
}

const std::array<Particle, kMaxParticles>& Game::particles() const noexcept {
    return particles_;
}

void Game::spawnExplosion(Vec2 pos, int count) {
    std::minstd_rand rng(particleSeed_);
    particleSeed_ += static_cast<std::uint32_t>(count);
    const auto randFloat = [&rng](float lo, float hi) -> float {
        const float frac = static_cast<float>(rng()) /
                           static_cast<float>(std::minstd_rand::max());
        return lo + (frac * (hi - lo));
    };
    int spawned = 0;
    for (auto& p : particles_) {
        if (spawned >= count) { break; }
        if (p.active) { continue; }
        const float angle    = randFloat(0.0F, (2.0F * kPi));
        const float speed    = randFloat(kParticleSpeedMin * scale_, kParticleSpeedMax * scale_);
        p.position    = pos;
        p.velocity    = Vec2{std::sin(angle), -std::cos(angle)} * speed;
        p.lifetime    = randFloat(kParticleLifetimeMin, kParticleLifetimeMax);
        p.maxLifetime = p.lifetime;
        p.active      = true;
        ++spawned;
    }
}

void Game::tickParticles(float dt) {
    for (auto& p : particles_) {
        if (!p.active) { continue; }
        p.lifetime -= dt;
        if (p.lifetime <= 0.0F) {
            p.active = false;
            continue;
        }
        p.position = integratePosition(p.position, p.velocity, dt);
        p.position = wrapPosition(p.position, screenSize_);
    }
}

void Game::tickRespawn(float dt) {
    if (state_ != GameState::PlayerDead) { return; }
    respawnTimer_ -= dt;
    if (respawnTimer_ <= 0.0F) {
        if (lives_ <= 0) {
            state_          = GameState::GameOver;
            gameOverTimer_  = kGameOverDelay;
        } else {
            state_            = GameState::Playing;
            ship_.position    = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
            ship_.velocity    = {0.0F, 0.0F};
            ship_.angle       = 0.0F;
            ship_.invincTimer = kInvincDuration;
            ship_.active      = true;
        }
    }
}

void Game::tickGameOver(float dt) {
    if (state_ != GameState::GameOver) { return; }
    gameOverTimer_ -= dt;
    if (gameOverTimer_ <= 0.0F) {
        scoreTable_.submit(score_);
        state_ = GameState::Attract;
    }
}

void Game::render(IRenderer& renderer) const {
    if (ship_.active) {
        const bool showShip =
            (ship_.invincTimer <= 0.0F) ||
            (std::fmod(ship_.invincTimer, (2.0F * kFlashPeriod)) < kFlashPeriod);
        if (showShip) {
            renderer.drawLineStrip(
                buildShipVertices(ship_.position, ship_.angle, scale_),
                Colour{255, 255, 255, 255},
                true);
        }
    }
    for (const auto& rock : asteroids_) {
        if (!rock.active || rock.shape.empty()) { continue; }
        renderer.drawLineStrip(
            buildAsteroidVertices(rock),
            Colour{255, 255, 255, 255},
            true);
    }
    if (saucer_.active) {
        const float r = Saucer::radius(saucer_.size) * scale_;
        renderer.drawLineStrip(scaledVerts(kSaucerBody, saucer_.position, r),
                               Colour{255, 255, 255, 255}, true);
        renderer.drawLineStrip(scaledVerts(kSaucerDome, saucer_.position, r),
                               Colour{255, 255, 255, 255}, false);
    }
    for (const auto& p : projectiles_) {
        if (!p.active) { continue; }
        const Vec2 dir = p.velocity.normalised();
        const float phl = kProjectileHalfLen * scale_;
        const Vec2  a{p.position.x - (dir.x * phl), p.position.y - (dir.y * phl)};
        const Vec2  b{p.position.x + (dir.x * phl), p.position.y + (dir.y * phl)};
        renderer.drawLine(a, b, Colour{255, 255, 255, 255});
    }

    for (const auto& p : particles_) {
        if (!p.active) { continue; }
        const auto  alpha = static_cast<std::uint8_t>(255.0F * (p.lifetime / p.maxLifetime));
        const Vec2  dir   = p.velocity.normalised();
        const float pthl = kParticleHalfLen * scale_;
        const Vec2  pa{p.position.x - (dir.x * pthl), p.position.y - (dir.y * pthl)};
        const Vec2  pb{p.position.x + (dir.x * pthl), p.position.y + (dir.y * pthl)};
        renderer.drawLine(pa, pb, Colour{255, 255, 255, alpha});
    }

    if (state_ == GameState::Playing || state_ == GameState::PlayerDead) {
        const float charH = screenSize_.y * kHudCharHeightFrac;

        // Score — top-left
        const float           hudMargin = kHudMargin * scale_;
        const std::string scoreStr = std::to_string(score_);
        drawString(renderer,
                   Vec2{hudMargin, hudMargin},
                   charH, scoreStr, Colour{255, 255, 255, 255});

        // Lives icons — top-right, small ship silhouettes pointing up
        const float iconScale = charH / kShipIconSpan;
        const float iconStep  = charH;
        for (int i = 0; i < lives_; ++i) {
            const float cx = screenSize_.x - hudMargin
                             - (charH * kIconHalfWidthFrac)
                             - (static_cast<float>(i) * iconStep);
            const float cy = hudMargin + (charH * kIconNoseFrac);
            renderer.drawLineStrip(
                buildShipVertices({cx, cy}, 0.0F, iconScale),
                Colour{255, 255, 255, 255},
                true);
        }
    }

    if (state_ == GameState::Attract) {
        const float titleCharH = screenSize_.y * kAttractTitleCharHeightFrac;
        const float titleW     = stringWidth(titleCharH, "ASTEROIDS");
        drawString(renderer,
                   Vec2{((screenSize_.x - titleW) * 0.5F), (screenSize_.y * kAttractTitleYFrac)},
                   titleCharH, "ASTEROIDS", Colour{255, 255, 255, 255});

        const float promptCharH = screenSize_.y * kAttractPromptCharHeightFrac;
        const float promptW     = stringWidth(promptCharH, "PRESS START");
        drawString(renderer,
                   Vec2{((screenSize_.x - promptW) * 0.5F), (screenSize_.y * kAttractPromptYFrac)},
                   promptCharH, "PRESS START", Colour{255, 255, 255, 255});

        const float scoreCharH = screenSize_.y * kAttractScoreCharHeightFrac;
        const float scoreLineH = screenSize_.y * kAttractScoreLineHeightFrac;
        for (std::size_t i = 0U; i < scoreTable_.count(); ++i) {
            const std::string s = std::to_string(scoreTable_.entry(i));
            const float       w = stringWidth(scoreCharH, s);
            const float       y = (screenSize_.y * kAttractScoreStartYFrac) +
                                  (static_cast<float>(i) * scoreLineH);
            drawString(renderer,
                       Vec2{((screenSize_.x - w) * 0.5F), y},
                       scoreCharH, s, Colour{255, 255, 255, 255});
        }
    }

    if (state_ == GameState::GameOver) {
        const float charH     = screenSize_.y * 0.08F;
        const float goW       = stringWidth(charH, "GAME OVER");
        drawString(renderer,
                   Vec2{((screenSize_.x - goW) * 0.5F), (screenSize_.y * 0.30F)},
                   charH, "GAME OVER", Colour{255, 255, 255, 255});

        const float scoreCharH = screenSize_.y * 0.05F;
        const std::string scoreStr = std::to_string(score_);
        const float scoreW = stringWidth(scoreCharH, scoreStr);
        drawString(renderer,
                   Vec2{((screenSize_.x - scoreW) * 0.5F), (screenSize_.y * 0.44F)},
                   scoreCharH, scoreStr, Colour{255, 255, 255, 255});

        const float tableCharH  = screenSize_.y * kAttractScoreCharHeightFrac;
        const float tableLineH  = screenSize_.y * kAttractScoreLineHeightFrac;
        for (std::size_t i = 0U; i < scoreTable_.count(); ++i) {
            const std::string s = std::to_string(scoreTable_.entry(i));
            const float       w = stringWidth(tableCharH, s);
            const float       y = (screenSize_.y * 0.56F) +
                                  (static_cast<float>(i) * tableLineH);
            drawString(renderer,
                       Vec2{((screenSize_.x - w) * 0.5F), y},
                       tableCharH, s, Colour{255, 255, 255, 255});
        }
    }
}

void Game::tickWave(float dt) {
    if (state_ != GameState::Playing) { return; }

    const bool allClear = std::none_of(asteroids_.begin(), asteroids_.end(),
                                        [](const Asteroid& a) { return a.active; });

    if (!prevAllClear_ && allClear) {
        interWaveTimer_ = kInterWaveDelay;
    }
    prevAllClear_ = allClear;

    if (interWaveTimer_ > 0.0F) { interWaveTimer_ -= dt; }

    if (allClear && interWaveTimer_ <= 0.0F) {
        ++waveNumber_;
        startWave();
        prevAllClear_ = false;
    }
}

void Game::startWave() {
    beatTimer_ = 0.0F;
    beatPhase_ = false;
    const int count = waveAsteroidCount(waveNumber_);
    std::minstd_rand rng(waveSeed_);

    const auto randFloat = [&rng](float lo, float hi) -> float {
        const float frac = static_cast<float>(rng()) / static_cast<float>(std::minstd_rand::max());
        return lo + (frac * (hi - lo));
    };

    for (int i = 0; i < count; ++i) {
        Asteroid rock;

        const auto edgeIdx = rng() % 4U;
        if (edgeIdx == 0U) {
            rock.position = {randFloat(0.0F, screenSize_.x), 0.0F};
        } else if (edgeIdx == 1U) {
            rock.position = {screenSize_.x, randFloat(0.0F, screenSize_.y)};
        } else if (edgeIdx == 2U) {
            rock.position = {randFloat(0.0F, screenSize_.x), screenSize_.y};
        } else {
            rock.position = {0.0F, randFloat(0.0F, screenSize_.y)};
        }

        const float angle = randFloat(0.0F, (2.0F * kPi));
        const float speed = randFloat(kAsteroidMinSpeed * scale_, kAsteroidMaxSpeed * scale_);
        const float sinA  = std::sin(angle);
        const float cosA  = std::cos(angle);
        rock.velocity     = Vec2{sinA, -cosA} * speed;
        rock.angularVel   = randFloat(-kAsteroidAngularVelMax, kAsteroidAngularVelMax);
        rock.size         = AsteroidSize::Large;
        rock.shape        = generateAsteroidShape(AsteroidSize::Large, 10,
                                                   waveSeed_ + static_cast<std::uint32_t>(i),
                                                   scale_);
        rock.active       = true;
        asteroids_.push_back(rock);
    }
    waveSeed_ += static_cast<std::uint32_t>(count);
}

GameState Game::state() const noexcept { return state_; }

const ScoreTable& Game::scoreTable() const noexcept { return scoreTable_; }

const Saucer& Game::saucer() const noexcept { return saucer_; }

const Ship& Game::ship() const noexcept { return ship_; }

int Game::score() const noexcept { return score_; }

int Game::waveNumber() const noexcept { return waveNumber_; }

int Game::lives() const noexcept { return lives_; }

void Game::tickBeat(float dt) {
    if (state_ != GameState::Playing) { return; }
    const int count = static_cast<int>(std::count_if(
        asteroids_.begin(), asteroids_.end(),
        [](const Asteroid& a) { return a.active; }));
    if (count == 0) { return; }
    beatTimer_ -= dt;
    if (beatTimer_ > 0.0F) { return; }
    audio_.play(beatPhase_ ? SoundId::BeatHigh : SoundId::BeatLow);
    beatPhase_ = !beatPhase_;
    beatTimer_ = beatInterval(count);
}

} // namespace ast
