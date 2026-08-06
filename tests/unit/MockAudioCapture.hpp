#pragma once
#include <gmock/gmock.h>
#include "pitch/IAudioCapture.hpp"

namespace ast {

class MockAudioCapture : public IAudioCapture {
public:
    MOCK_METHOD(void, readSamples, (std::vector<float>& out), (override));
    MOCK_METHOD(int,  sampleRate,  (), (const, override));
    MOCK_METHOD(bool, isOk,        (), (const, override));
};

} // namespace ast
