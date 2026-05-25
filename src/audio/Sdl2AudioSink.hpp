#pragma once
#include "game/IAudioSink.hpp"

namespace ast {

class Sdl2AudioSink : public IAudioSink {
public:
    Sdl2AudioSink();
    ~Sdl2AudioSink() override;

    Sdl2AudioSink(const Sdl2AudioSink&)            = delete;
    Sdl2AudioSink& operator=(const Sdl2AudioSink&) = delete;
    Sdl2AudioSink(Sdl2AudioSink&&)                 = delete;
    Sdl2AudioSink& operator=(Sdl2AudioSink&&)      = delete;

    void play(SoundId id) override;
    void loop(SoundId id) override;
    void stop(SoundId id) override;
    bool isPlaying(SoundId id) const override;

private:
    bool initialised_ = false;
};

} // namespace ast
