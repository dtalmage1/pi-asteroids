#pragma once
#include <vector>

namespace ast {

class IAudioCapture {
public:
    virtual ~IAudioCapture() = default;
    IAudioCapture()                                = default;
    IAudioCapture(const IAudioCapture&)            = delete;
    IAudioCapture& operator=(const IAudioCapture&) = delete;
    IAudioCapture(IAudioCapture&&)                 = delete;
    IAudioCapture& operator=(IAudioCapture&&)      = delete;

    // Appends newly captured mono samples (range [-1, 1]) to the end of out.
    virtual void readSamples(std::vector<float>& out) = 0;
    virtual int  sampleRate() const = 0;
    virtual bool isOk() const = 0;
};

} // namespace ast
