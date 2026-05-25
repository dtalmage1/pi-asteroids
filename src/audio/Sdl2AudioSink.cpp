#include "audio/Sdl2AudioSink.hpp"
#include <SDL_mixer.h>

namespace ast {

Sdl2AudioSink::Sdl2AudioSink() {
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
        initialised_ = true;
    }
}

Sdl2AudioSink::~Sdl2AudioSink() {
    if (initialised_) {
        Mix_CloseAudio();
    }
}

void Sdl2AudioSink::play(SoundId /*id*/) {}
void Sdl2AudioSink::loop(SoundId /*id*/) {}
void Sdl2AudioSink::stop(SoundId /*id*/) {}
bool Sdl2AudioSink::isPlaying(SoundId /*id*/) const { return false; }

} // namespace ast
