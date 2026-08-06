#include <gtest/gtest.h>
#include <vector>
#include "MockAudioCapture.hpp"

// MockAudioCapture compiles with GoogleMock and its methods are callable.
// WillOnce avoided: see MockInputSource tests for rationale.
TEST(MockAudioCapture, ReadSamplesCanBeCalled) {
    ast::MockAudioCapture mock;
    std::vector<float> buffer;
    EXPECT_CALL(mock, readSamples(testing::_)).Times(1);
    mock.readSamples(buffer);
}

TEST(MockAudioCapture, SampleRateCanBeCalled) {
    ast::MockAudioCapture mock;
    EXPECT_CALL(mock, sampleRate()).Times(1);
    mock.sampleRate();
}

TEST(MockAudioCapture, IsOkCanBeCalled) {
    ast::MockAudioCapture mock;
    EXPECT_CALL(mock, isOk()).Times(1);
    mock.isOk();
}
