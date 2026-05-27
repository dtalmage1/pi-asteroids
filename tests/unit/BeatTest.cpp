#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
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

ast::Asteroid largeAsteroidAt(ast::Vec2 pos) {
    ast::Asteroid rock;
    rock.position   = pos;
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Large;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 1U);
    rock.active     = true;
    return rock;
}

// Safe spawn position: far corner, well away from ship at screen centre (400,300).
constexpr ast::Vec2 kSafePos{50.0F, 50.0F};

} // namespace

// BeatLow plays at least once while an asteroid is active.
TEST(Beat, BeatLowPlaysDuringWave) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.spawnAsteroid(largeAsteroidAt(kSafePos));

    EXPECT_CALL(audio, play(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::BeatLow)).Times(testing::AtLeast(1));

    tickFrames(game, 60);  // 1 s — covers at least one beat at max interval (0.9 s)
}

// Beat alternates: BeatLow fires first, then BeatHigh.
TEST(Beat, BeatAlternatesLowHigh) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    game.spawnAsteroid(largeAsteroidAt(kSafePos));

    EXPECT_CALL(audio, play(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::BeatHigh)).Times(testing::AtLeast(1));
    EXPECT_CALL(audio, play(ast::SoundId::BeatLow)).Times(testing::AtLeast(1));

    tickFrames(game, 120);  // 2 s — enough for two beats at max interval
}

// No beat when no asteroids are active (inter-wave gap).
TEST(Beat, NoBeatWhenWaveClear) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);
    // No asteroids spawned; game is Playing but wave not yet started

    EXPECT_CALL(audio, play(ast::SoundId::BeatLow)).Times(0);
    EXPECT_CALL(audio, play(ast::SoundId::BeatHigh)).Times(0);

    tickFrames(game, 60);  // 1 s — well within the 2 s inter-wave delay
}

// No beat while game is in Attract state.
TEST(Beat, NoBeatInAttract) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    // Game starts in Attract; never call startGame

    EXPECT_CALL(audio, play(ast::SoundId::BeatLow)).Times(0);
    EXPECT_CALL(audio, play(ast::SoundId::BeatHigh)).Times(0);

    tickFrames(game, 60);
}
