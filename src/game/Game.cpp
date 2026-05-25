#include "game/Game.hpp"
#include "game/Colour.hpp"
#include "game/physics/Collision.hpp"
#include "game/physics/Physics.hpp"
#include <array>
#include <cmath>
#include <vector>

namespace {
constexpr float kShipDrag           = 0.5F;
constexpr float kRotationRate       = 3.5F;   // rad/s (~200 deg/s)
constexpr float kThrustAccel        = 200.0F; // pixels/s²
constexpr float kProjectileSpeed    = 500.0F; // pixels/s
constexpr float kProjectileLifetime = 0.75F;  // seconds
constexpr float kShipNoseOffset     = 15.0F;  // pixels from ship centre to nose tip
constexpr float kProjectileHalfLen  =  2.0F;  // half visual length of projectile line
constexpr float kProjectileRadius   =  2.0F;  // collision radius (matches visual size)
constexpr float kSplitAngle         =  0.5236F; // π/6 ≈ 30° divergence per child
constexpr float kSplitSpeedMult     =  1.3F;    // children are 30% faster than parent
constexpr float kSplitMinSpeed      = 60.0F;    // floor speed for stationary parent (pixels/s)
constexpr float kSplitAngularMult   =  1.5F;    // children spin faster than parent
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
            pos.x + (v.x * cosA) - (v.y * sinA),
            pos.y + (v.x * sinA) + (v.y * cosA)
        });
    }
    return verts;
}

std::vector<ast::Asteroid> spawnChildren(const ast::Asteroid& parent, uint32_t seed) {
    if (parent.size == ast::AsteroidSize::Small) { return {}; }
    const ast::AsteroidSize childSize = (parent.size == ast::AsteroidSize::Large)
                                        ? ast::AsteroidSize::Medium
                                        : ast::AsteroidSize::Small;
    const float parentSpeed = parent.velocity.length();
    const float childSpeed  = std::max((parentSpeed * kSplitSpeedMult), kSplitMinSpeed);
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
    c0.shape      = ast::generateAsteroidShape(childSize, 10, seed);
    c0.active     = true;
    children.push_back(std::move(c0));
    ast::Asteroid c1;
    c1.position   = parent.position;
    c1.velocity   = vel2;
    c1.angularVel = parent.angularVel * kSplitAngularMult;
    c1.size       = childSize;
    c1.shape      = ast::generateAsteroidShape(childSize, 10, seed + 1U);
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
    : audio_(audio), screenSize_(screenSize)
{
    ship_.position = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
}

void Game::update(float dt, const InputState& input) {
    if (state_ == GameState::Attract && input.start) {
        state_         = GameState::Playing;
        ship_.position = {screenSize_.x / 2.0F, screenSize_.y / 2.0F};
        ship_.velocity = {0.0F, 0.0F};
        ship_.angle    = 0.0F;
        ship_.active   = true;
    }

    if (state_ == GameState::Playing && ship_.active) {
        if (input.rotateLeft)  { ship_.angle -= kRotationRate * dt; }
        if (input.rotateRight) { ship_.angle += kRotationRate * dt; }
        ship_.angle = wrapAngle(ship_.angle);

        ship_.thrusting = input.thrust;
        if (input.thrust) {
            ship_.velocity = applyThrust(ship_.velocity, ship_.angle, kThrustAccel, dt);
        }

        ship_.velocity = applyDrag(ship_.velocity, kShipDrag, dt);
        ship_.position = integratePosition(ship_.position, ship_.velocity, dt);
        ship_.position = wrapPosition(ship_.position, screenSize_);

        if (ship_.invincTimer > 0.0F) { ship_.invincTimer -= dt; }

        tryFire(input);
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
    checkShipCollisions();
}

void Game::tryFire(const InputState& input) {
    if (!input.fire) { return; }
    Projectile* slot = nullptr;
    for (auto& candidate : projectiles_) {
        if (!candidate.active) { slot = &candidate; break; }
    }
    if (slot != nullptr) {
        const Vec2 nose{std::sin(ship_.angle), -std::cos(ship_.angle)};
        slot->position = ship_.position + (nose * kShipNoseOffset);
        slot->velocity = ship_.velocity + (nose * kProjectileSpeed);
        slot->lifetime = kProjectileLifetime;
        slot->owner    = ProjectileOwner::Player;
        slot->active   = true;
    }
}

void Game::checkCollisions() {
    std::vector<Asteroid> spawned;
    for (auto& p : projectiles_) {
        if (!p.active || p.owner != ProjectileOwner::Player) { continue; }
        for (auto& rock : asteroids_) {
            if (!rock.active) { continue; }
            if (circlesOverlap(p.position, kProjectileRadius,
                               rock.position, Asteroid::radius(rock.size))) {
                p.active      = false;
                auto children = spawnChildren(rock, splitSeed_);
                splitSeed_   += 2U;
                for (auto& child : children) { spawned.push_back(std::move(child)); }
                rock.active   = false;
                break;
            }
        }
    }
    for (auto& a : spawned) { asteroids_.push_back(std::move(a)); }
}

void Game::checkShipCollisions() {
    if (state_ != GameState::Playing) { return; }
    if (!ship_.active || ship_.invincTimer > 0.0F) { return; }
    for (const auto& rock : asteroids_) {
        if (!rock.active) { continue; }
        if (circlesOverlap(ship_.position, Ship::kRadius,
                           rock.position, Asteroid::radius(rock.size))) {
            ship_.active = false;
            state_       = GameState::PlayerDead;
            return;
        }
    }
}

void Game::spawnAsteroid(Asteroid a) {
    asteroids_.push_back(std::move(a));
}

const std::vector<Asteroid>& Game::asteroids() const noexcept {
    return asteroids_;
}

const std::array<Projectile, kMaxProjectiles>& Game::projectiles() const noexcept {
    return projectiles_;
}

void Game::render(IRenderer& renderer) const {
    if (ship_.active) {
        renderer.drawLineStrip(
            buildShipVertices(ship_.position, ship_.angle),
            Colour{255, 255, 255, 255},
            true);
    }
    for (const auto& rock : asteroids_) {
        if (!rock.active || rock.shape.empty()) { continue; }
        renderer.drawLineStrip(
            buildAsteroidVertices(rock),
            Colour{255, 255, 255, 255},
            true);
    }
    for (const auto& p : projectiles_) {
        if (!p.active) { continue; }
        const Vec2 dir = p.velocity.normalised();
        const Vec2 a{p.position.x - (dir.x * kProjectileHalfLen),
                     p.position.y - (dir.y * kProjectileHalfLen)};
        const Vec2 b{p.position.x + (dir.x * kProjectileHalfLen),
                     p.position.y + (dir.y * kProjectileHalfLen)};
        renderer.drawLine(a, b, Colour{255, 255, 255, 255});
    }
}

GameState Game::state() const noexcept { return state_; }

const Ship& Game::ship() const noexcept { return ship_; }

} // namespace ast
