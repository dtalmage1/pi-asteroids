#include "audio/Sdl2AudioSink.hpp"
#include "game/SoundId.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <array>
#include <iostream>
#include <memory>
#include <string>

namespace {

// WAV filename for each SoundId (index = static_cast<int>(SoundId)).
constexpr std::array<const char*, 10> kFiles{{
    "thrust.wav",       // 0  SoundId::Thrust
    "fire.wav",         // 1  SoundId::Fire
    "bangSmall.wav",    // 2  SoundId::ExplosionSmall
    "bangLarge.wav",    // 3  SoundId::ExplosionLarge
    "saucerBig.wav",    // 4  SoundId::SaucerEngine
    "saucer_fire.wav",  // 5  SoundId::SaucerFire
    "beat1.wav",        // 6  SoundId::BeatLow
    "beat2.wav",        // 7  SoundId::BeatHigh
    "bangMedium.wav",   // 8  SoundId::ExplosionMedium
    "saucerSmall.wav",  // 9  SoundId::SaucerEngineSmall
}};

struct SdlStringDeleter {
    // SDL_free maps to the C free(); suppressed here because SDL ownership
    // semantics require it for strings returned by SDL_GetBasePath.
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
    void operator()(char* p) const noexcept { SDL_free(p); }
};

} // namespace

namespace ast {

int Sdl2AudioSink::soundIndex(SoundId id) noexcept {
    return static_cast<int>(id);
}

Sdl2AudioSink::Sdl2AudioSink() {
    chunks_.fill(nullptr);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << '\n';
        return;
    }
    Mix_AllocateChannels(static_cast<int>(kSoundCount));
    initialised_ = true;

    // Locate assets next to the executable (CMake copies them there at build time).
    std::string audioDir;
    {
        std::unique_ptr<char, SdlStringDeleter> base(SDL_GetBasePath());
        audioDir = base ? std::string(base.get()) : "./";
    }
    audioDir += "assets/audio/";

    for (std::size_t i = 0; i < kSoundCount; ++i) {
        const std::string path = audioDir + kFiles.at(i);
        chunks_.at(i) = Mix_LoadWAV(path.c_str());
        if (chunks_.at(i) == nullptr) {
            std::cerr << "Mix_LoadWAV(" << path << "): " << Mix_GetError() << '\n';
        }
    }
}

Sdl2AudioSink::~Sdl2AudioSink() {
    if (initialised_) {
        Mix_HaltChannel(-1);
        for (auto* chunk : chunks_) {
            if (chunk != nullptr) { Mix_FreeChunk(chunk); }
        }
        Mix_CloseAudio();
    }
}

void Sdl2AudioSink::play(SoundId id) {
    const auto idx = static_cast<std::size_t>(soundIndex(id));
    if (!initialised_ || chunks_.at(idx) == nullptr) { return; }
    Mix_PlayChannel(static_cast<int>(idx), chunks_.at(idx), 0);
}

void Sdl2AudioSink::loop(SoundId id) {
    const auto idx = static_cast<std::size_t>(soundIndex(id));
    if (!initialised_ || chunks_.at(idx) == nullptr) { return; }
    if (Mix_Playing(static_cast<int>(idx)) == 0) {
        Mix_PlayChannel(static_cast<int>(idx), chunks_.at(idx), -1);
    }
}

void Sdl2AudioSink::stop(SoundId id) {
    if (!initialised_) { return; }
    Mix_HaltChannel(soundIndex(id));
}

bool Sdl2AudioSink::isPlaying(SoundId id) const {
    if (!initialised_) { return false; }
    return Mix_Playing(soundIndex(id)) != 0;
}

} // namespace ast
