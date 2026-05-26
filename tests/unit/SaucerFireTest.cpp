#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
#include "MockAudioSink.hpp"

namespace {

constexpr int kSpawnFrames = 920;  // > 15 s at 60 Hz — saucer is active after this
constexpr int kFireFrames  =  96;  // > 1.5 s * 60 Hz — saucer has fired at least once

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

void tickFrames(ast::Game& game, int n) {
    const ast::InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
}

ast::Projectile playerProjectileAt(ast::Vec2 pos) {
    ast::Projectile p;
    p.position = pos;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Player;
    p.active    = true;
    return p;
}

} // namespace

// Saucer fires a projectile with ProjectileOwner::Saucer while active.
TEST(SaucerFire, FiresProjectileWhileActive) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, kSpawnFrames);
    ASSERT_TRUE(game.saucer().active);
    tickFrames(game, kFireFrames);

    bool found = false;
    for (const auto& p : game.projectiles()) {
        if (p.active && p.owner == ast::ProjectileOwner::Saucer) { found = true; }
    }
    EXPECT_TRUE(found);
}

// A player projectile overlapping the saucer deactivates it.
TEST(SaucerFire, PlayerProjectileDestroySaucer) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, kSpawnFrames);
    ASSERT_TRUE(game.saucer().active);

    game.spawnProjectile(playerProjectileAt(game.saucer().position));
    tickFrames(game, 1);

    EXPECT_FALSE(game.saucer().active);
}

// Destroying the saucer awards 200 points.
TEST(SaucerFire, DestroyAwards200Points) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, kSpawnFrames);
    ASSERT_TRUE(game.saucer().active);

    game.spawnProjectile(playerProjectileAt(game.saucer().position));
    tickFrames(game, 1);

    EXPECT_EQ(game.score(), 200);
}

// A saucer-owned projectile next to an asteroid does not destroy it.
TEST(SaucerFire, SaucerProjectileDoesNotHitAsteroid) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);  // just get into Playing state

    constexpr ast::Vec2 kTarget{200.0F, 200.0F};

    ast::Asteroid rock;
    rock.position = kTarget;
    rock.size     = ast::AsteroidSize::Large;
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::Projectile p;
    p.position = kTarget;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Saucer;
    p.active    = true;
    game.spawnProjectile(p);

    tickFrames(game, 1);

    bool asteroidSurvived = false;
    for (const auto& a : game.asteroids()) {
        if (a.active && a.position.x == kTarget.x) { asteroidSurvived = true; }
    }
    EXPECT_TRUE(asteroidSurvived);
}
