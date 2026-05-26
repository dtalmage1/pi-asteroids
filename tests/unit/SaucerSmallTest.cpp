#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/entities/Saucer.hpp"
#include "MockAudioSink.hpp"

namespace {

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

void tickFrames(ast::Game& game, int n) {
    const ast::InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
}

ast::Projectile makeProjectile(ast::Vec2 pos, ast::ProjectileOwner owner) {
    ast::Projectile p;
    p.position = pos;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = owner;
    p.active    = true;
    return p;
}

} // namespace

// A saucer-owned projectile overlapping the ship kills it.
TEST(SaucerVsShip, SaucerProjectileKillsShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);

    game.spawnProjectile(makeProjectile(game.ship().position, ast::ProjectileOwner::Saucer));
    tickFrames(game, 1);

    EXPECT_EQ(game.state(), ast::GameState::PlayerDead);
}

// The saucer body overlapping the ship kills it.
TEST(SaucerVsShip, SaucerBodyKillsShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);

    game.activateSaucer(ast::SaucerSize::Large, game.ship().position);
    tickFrames(game, 1);

    EXPECT_EQ(game.state(), ast::GameState::PlayerDead);
}

// Destroying a small saucer with a player projectile awards 1000 points.
TEST(SaucerVsShip, SmallSaucerAwards1000Points) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);

    constexpr ast::Vec2 kPos{300.0F, 300.0F};
    game.activateSaucer(ast::SaucerSize::Small, kPos);
    game.spawnProjectile(makeProjectile(kPos, ast::ProjectileOwner::Player));
    tickFrames(game, 1);

    EXPECT_EQ(game.score(), 1000);
}

// An invincible ship (freshly respawned) survives a saucer projectile.
TEST(SaucerVsShip, InvincibleShipSurvivesSaucerProjectile) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);

    // Kill the ship to enter PlayerDead, then wait for respawn (invincTimer > 0).
    game.spawnProjectile(makeProjectile(game.ship().position, ast::ProjectileOwner::Saucer));
    tickFrames(game, 1);
    ASSERT_EQ(game.state(), ast::GameState::PlayerDead);

    tickFrames(game, 181);  // 3 s * 60 Hz + 1 buffer → ship respawns with invincTimer
    ASSERT_EQ(game.state(), ast::GameState::Playing);
    ASSERT_GT(game.ship().invincTimer, 0.0F);

    game.spawnProjectile(makeProjectile(game.ship().position, ast::ProjectileOwner::Saucer));
    tickFrames(game, 1);

    EXPECT_EQ(game.state(), ast::GameState::Playing);
}

// An invincible ship survives a saucer body collision.
TEST(SaucerVsShip, InvincibleShipSurvivesSaucerBody) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    tickFrames(game, 2);

    // Kill and respawn to gain invincibility.
    game.spawnProjectile(makeProjectile(game.ship().position, ast::ProjectileOwner::Saucer));
    tickFrames(game, 1);
    ASSERT_EQ(game.state(), ast::GameState::PlayerDead);

    tickFrames(game, 181);
    ASSERT_EQ(game.state(), ast::GameState::Playing);
    ASSERT_GT(game.ship().invincTimer, 0.0F);

    game.activateSaucer(ast::SaucerSize::Large, game.ship().position);
    tickFrames(game, 1);

    EXPECT_EQ(game.state(), ast::GameState::Playing);
}
