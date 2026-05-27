#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/entities/Projectile.hpp"
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

// Saucer engine loops every frame while the saucer is active.
TEST(SaucerAudio, EngineLoopsWhileActive) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.activateSaucer(ast::SaucerSize::Large, {400.0F, 300.0F});

    EXPECT_CALL(audio, loop(ast::SoundId::SaucerEngine)).Times(testing::AtLeast(1));

    tickFrames(game, 1);
}

// Saucer engine stops when the saucer exits the screen boundary.
TEST(SaucerAudio, EngineStopsOnScreenExit) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    // Position past the right edge so the exit check triggers immediately.
    game.activateSaucer(ast::SaucerSize::Large, {820.0F, 300.0F});

    // Declare catch-all first so the specific expectation (LIFO) is tried first.
    EXPECT_CALL(audio, stop(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, stop(ast::SoundId::SaucerEngine)).Times(1);

    tickFrames(game, 1);
}

// Saucer engine stops when the saucer is destroyed by a player projectile.
TEST(SaucerAudio, EngineStopsWhenShot) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.activateSaucer(ast::SaucerSize::Large, {400.0F, 200.0F});
    game.spawnProjectile(playerProjectileAt({400.0F, 200.0F}));

    EXPECT_CALL(audio, stop(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, stop(ast::SoundId::SaucerEngine)).Times(1);

    tickFrames(game, 1);
}

// Small saucer engine loops SaucerEngineSmall (not SaucerEngine).
TEST(SaucerAudio, SmallSaucerLoopsSmallEngineSound) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.activateSaucer(ast::SaucerSize::Small, {400.0F, 300.0F});

    EXPECT_CALL(audio, loop(ast::SoundId::SaucerEngineSmall)).Times(testing::AtLeast(1));

    tickFrames(game, 1);
}

// Saucer firing plays the SaucerFire sound.
TEST(SaucerAudio, FirePlaysSFX) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.activateSaucer(ast::SaucerSize::Large, {400.0F, 200.0F});

    // Allow any other audio calls; verify SaucerFire fires at least once.
    EXPECT_CALL(audio, play(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::SaucerFire)).Times(testing::AtLeast(1));

    // 1.5 s fire interval + 1 frame margin = 92 frames at 60 Hz.
    tickFrames(game, 92);
}
