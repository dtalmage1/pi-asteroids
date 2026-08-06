#include "audio/Sdl2AudioCapture.hpp"
#include <SDL.h>
#include <iostream>

namespace ast {

// NOLINTNEXTLINE(readability-non-const-parameter) signature is fixed by SDL_AudioCallback
void Sdl2AudioCapture::audioCallback(void* userdata, std::uint8_t* stream, int lengthBytes) {
    auto* self = static_cast<Sdl2AudioCapture*>(userdata);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) SDL hands us a raw byte
    // buffer; AUDIO_F32SYS guarantees it holds tightly-packed native-endian floats.
    const auto* samples = reinterpret_cast<const float*>(stream);
    const auto  count   = static_cast<std::size_t>(lengthBytes) / sizeof(float);
    self->handleCallback(samples, count);
}

void Sdl2AudioCapture::handleCallback(const float* samples, std::size_t count) {
    const std::lock_guard<std::mutex> lock(mutex_);
    pending_.insert(pending_.end(), samples, samples + count);
}

Sdl2AudioCapture::Sdl2AudioCapture(int sampleRate, int bufferSamples)
    : sampleRate_(sampleRate)
{
    SDL_AudioSpec desired{};
    desired.freq     = sampleRate;
    desired.format   = AUDIO_F32SYS;
    desired.channels = 1;
    desired.samples  = static_cast<Uint16>(bufferSamples);
    desired.callback = &Sdl2AudioCapture::audioCallback;
    desired.userdata = this;

    SDL_AudioSpec obtained{};
    // allowed_changes = 0: SDL performs any needed conversion internally so
    // obtained always matches desired, keeping downstream code format-agnostic.
    deviceId_ = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &desired, &obtained, 0);
    if (deviceId_ == 0) {
        std::cerr << "SDL_OpenAudioDevice (capture) failed: " << SDL_GetError() << '\n';
        return;
    }
    sampleRate_ = obtained.freq;
    ok_         = true;
    SDL_PauseAudioDevice(deviceId_, 0); // unpause: start capturing
}

Sdl2AudioCapture::~Sdl2AudioCapture() {
    if (deviceId_ != 0) {
        SDL_CloseAudioDevice(deviceId_);
    }
}

void Sdl2AudioCapture::readSamples(std::vector<float>& out) {
    const std::lock_guard<std::mutex> lock(mutex_);
    out.insert(out.end(), pending_.begin(), pending_.end());
    pending_.clear();
}

int Sdl2AudioCapture::sampleRate() const {
    return sampleRate_;
}

bool Sdl2AudioCapture::isOk() const {
    return ok_;
}

} // namespace ast
