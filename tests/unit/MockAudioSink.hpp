#pragma once
#include <gmock/gmock.h>
#include "game/IAudioSink.hpp"

namespace ast {

class MockAudioSink : public IAudioSink {
public:
    MOCK_METHOD(void, play,      (SoundId id), (override));
    MOCK_METHOD(void, loop,      (SoundId id), (override));
    MOCK_METHOD(void, stop,      (SoundId id), (override));
    MOCK_METHOD(bool, isPlaying, (SoundId id), (const, override));
};

} // namespace ast
