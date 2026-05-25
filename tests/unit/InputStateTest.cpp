#include <gtest/gtest.h>
#include "game/InputState.hpp"
#include "MockInputSource.hpp"

// All InputState fields default to false (including connected).
TEST(InputState, DefaultsToAllFalse) {
    const ast::InputState s;
    EXPECT_FALSE(s.thrust);
    EXPECT_FALSE(s.rotateLeft);
    EXPECT_FALSE(s.rotateRight);
    EXPECT_FALSE(s.fire);
    EXPECT_FALSE(s.hyperspace);
    EXPECT_FALSE(s.start);
    EXPECT_FALSE(s.connected);
}

// MockInputSource compiles with GoogleMock and its query() method is callable.
// Note: WillOnce(Return(...)) / WillOnce(lambda) are avoided here because
// gmock's WillOnce instantiates PolymorphicAction machinery that clang-tidy >=19
// in GCC mode (Linux/RPi) cannot resolve. Times(N) assertions are sufficient to
// verify that the mock wires up correctly to the IInputSource interface.
TEST(MockInputSource, QueryCanBeCalled) {
    ast::MockInputSource mock;
    EXPECT_CALL(mock, query()).Times(1);
    const ast::InputState result = mock.query();
    // Default-constructed return value has all fields false.
    EXPECT_FALSE(result.connected);
}

// MockInputSource::query() can be expected and called multiple times.
TEST(MockInputSource, QueryCanBeCalledMultipleTimes) {
    ast::MockInputSource mock;
    EXPECT_CALL(mock, query()).Times(3);
    mock.query();
    mock.query();
    mock.query();
}
