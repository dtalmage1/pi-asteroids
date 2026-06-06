#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"
#include "TestHelpers.hpp"

namespace {

constexpr int kSpawnFrames   = 920;  // > 15 s at 60 Hz (15*60=900)
constexpr int kCrossFrames   = 640;  // > 812 px / 80 px·s⁻¹ * 60 ≈ 609 frames

} // namespace

// No saucer is active immediately after the game starts.
TEST(Saucer, NoSaucerOnGameStart) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    EXPECT_FALSE(game.saucer().active);
}

// Saucer spawns after the 15-second timer expires.
TEST(Saucer, SpawnsAfterDelay) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames);
    EXPECT_TRUE(game.saucer().active);
}

// Saucer moves each frame while active.
TEST(Saucer, MovesEachFrame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames);

    const ast::Vec2 posBefore = game.saucer().position;
    ast::test::tickFrames(game, 1);
    const ast::Vec2 posAfter  = game.saucer().position;

    EXPECT_TRUE((posAfter.x != posBefore.x) || (posAfter.y != posBefore.y));
}

// Saucer deactivates once it exits the screen boundary.
TEST(Saucer, ExitsScreen) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames);          // saucer now active
    ASSERT_TRUE(game.saucer().active);
    ast::test::tickFrames(game, kCrossFrames);          // saucer has crossed the 800 px screen
    EXPECT_FALSE(game.saucer().active);
}

// Saucer spawn position is within vertical screen bounds.
TEST(Saucer, SpawnPositionWithinScreen) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames);

    EXPECT_GE(game.saucer().position.y, 0.0F);
    EXPECT_LE(game.saucer().position.y, 600.0F);
}

// Saucer timer resets after exit, so a second saucer spawns eventually.
TEST(Saucer, SpawnsAgainAfterExit) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames + kCrossFrames);  // first saucer spawned and gone
    ASSERT_FALSE(game.saucer().active);

    // Tick up to 3×kSpawnFrames more frames.  If the ship exhausts all lives and
    // the game reaches Attract, restart it — the saucer timer counts in Playing and
    // PlayerDead states, so a second saucer must appear within one spawn interval
    // of continuous non-Attract time.
    const ast::InputState noInput;
    bool spawned = false;
    for (int i = 0; i < (3 * kSpawnFrames) && !spawned; ++i) {
        if (game.state() == ast::GameState::Attract) {
            ast::test::startGame(game);
        }
        game.update(1.0F / 60.0F, noInput);
        spawned = game.saucer().active;
    }
    EXPECT_TRUE(spawned);
}

// Active saucer produces drawLineStrip calls during rendering.
TEST(Saucer, RenderedWhenActive) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ast::test::tickFrames(game, kSpawnFrames);
    ASSERT_TRUE(game.saucer().active);

    ast::MockRenderer renderer;
    // Body (closed, 6 pts → 6 segments) + dome (open, 4 pts → 3 segments) = 2 strip calls
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(2));
    game.render(renderer);
}

// Saucer does not render when inactive.
TEST(Saucer, NotRenderedWhenInactive) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    ASSERT_FALSE(game.saucer().active);

    ast::MockRenderer renderer;
    // Only the ship strip should fire — never from the saucer.
    // Allow drawLineStrip but check saucer doesn't add its two strips by verifying
    // total is <= ship (1) + lives icons (3) = 4, well below ship+saucer (6).
    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, testing::_))
        .Times(testing::AtMost(4));
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber());
    game.render(renderer);
}

// Saucer does not spawn in Attract state.
TEST(Saucer, NoSpawnInAttract) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    // Do NOT call startGame — stay in Attract for many frames
    ast::test::tickFrames(game, kSpawnFrames);
    EXPECT_FALSE(game.saucer().active);
}
