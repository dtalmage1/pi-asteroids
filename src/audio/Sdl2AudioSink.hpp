#pragma once
#include "game/IAudioSink.hpp"
#include <array>

struct Mix_Chunk; // forward-declare to avoid pulling SDL_mixer.h into consumers

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
    static constexpr std::size_t kSoundCount = 10;
    static int soundIndex(SoundId id) noexcept;

    bool initialised_ = false;
    std::array<Mix_Chunk*, kSoundCount> chunks_{};
};

} // namespace ast
