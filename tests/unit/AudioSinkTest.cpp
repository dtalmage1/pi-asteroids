#include <gtest/gtest.h>
#include "game/NullAudioSink.hpp"
#include "MockAudioSink.hpp"

// NullAudioSink::isPlaying() always returns false for any SoundId.
TEST(NullAudioSink, IsPlayingReturnsFalse) {
    ast::NullAudioSink sink;
    EXPECT_FALSE(sink.isPlaying(ast::SoundId::Thrust));
    EXPECT_FALSE(sink.isPlaying(ast::SoundId::Fire));
    EXPECT_FALSE(sink.isPlaying(ast::SoundId::BeatHigh));
}

// NullAudioSink play/loop/stop are no-ops — callable without crash.
TEST(NullAudioSink, PlayLoopStopAreNoOps) {
    ast::NullAudioSink sink;
    sink.play(ast::SoundId::Fire);
    sink.loop(ast::SoundId::Thrust);
    sink.stop(ast::SoundId::Thrust);
}

// MockAudioSink compiles with GoogleMock and its methods are callable.
// WillOnce avoided: see MockInputSource tests for rationale.
TEST(MockAudioSink, PlayCanBeCalled) {
    ast::MockAudioSink mock;
    EXPECT_CALL(mock, play(ast::SoundId::Fire)).Times(1);
    mock.play(ast::SoundId::Fire);
}

TEST(MockAudioSink, IsPlayingCanBeCalled) {
    ast::MockAudioSink mock;
    EXPECT_CALL(mock, isPlaying(ast::SoundId::Thrust)).Times(1);
    mock.isPlaying(ast::SoundId::Thrust);
}
