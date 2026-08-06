#pragma once
#include "pitch/IAudioCapture.hpp"
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace ast {

// Concrete IAudioCapture backed by SDL2's raw audio capture device API
// (distinct from SDL2_mixer, which only plays sound). Opens the system
// default capture device (the USB microphone) as mono 32-bit float PCM.
class Sdl2AudioCapture : public IAudioCapture {
public:
    explicit Sdl2AudioCapture(int sampleRate = 48000, int bufferSamples = 1024);
    ~Sdl2AudioCapture() override;

    Sdl2AudioCapture(const Sdl2AudioCapture&)            = delete;
    Sdl2AudioCapture& operator=(const Sdl2AudioCapture&) = delete;
    Sdl2AudioCapture(Sdl2AudioCapture&&)                 = delete;
    Sdl2AudioCapture& operator=(Sdl2AudioCapture&&)      = delete;

    void readSamples(std::vector<float>& out) override;
    int  sampleRate() const override;
    bool isOk() const override;

private:
    static void audioCallback(void* userdata, std::uint8_t* stream, int lengthBytes);
    void handleCallback(const float* samples, std::size_t count);

    std::uint32_t deviceId_ = 0;
    int           sampleRate_;
    bool          ok_ = false;

    mutable std::mutex mutex_;
    std::vector<float> pending_;
};

} // namespace ast
