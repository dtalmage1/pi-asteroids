#pragma once
#include "game/IAudioSink.hpp"

namespace ast {

class NullAudioSink : public IAudioSink {
public:
    void play(SoundId /*id*/) override {}
    void loop(SoundId /*id*/) override {}
    void stop(SoundId /*id*/) override {}
    bool isPlaying(SoundId /*id*/) const override { return false; }
};

} // namespace ast
