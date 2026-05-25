#pragma once
#include "game/SoundId.hpp"

namespace ast {

class IAudioSink {
public:
    virtual ~IAudioSink() = default;
    IAudioSink()                               = default;
    IAudioSink(const IAudioSink&)             = delete;
    IAudioSink& operator=(const IAudioSink&)  = delete;
    IAudioSink(IAudioSink&&)                  = delete;
    IAudioSink& operator=(IAudioSink&&)       = delete;

    virtual void play(SoundId id) = 0;
    virtual void loop(SoundId id) = 0;
    virtual void stop(SoundId id) = 0;
    virtual bool isPlaying(SoundId id) const = 0;
};

} // namespace ast
