#include <gtest/gtest.h>
#include <cmath>
#include "game/Vec2.hpp"
#include "MockRenderer.hpp"

// Vec2 default constructor initialises both components to zero.
TEST(Vec2, DefaultConstructsToZero) {
    const ast::Vec2 v;
    EXPECT_FLOAT_EQ(v.x, 0.F);
    EXPECT_FLOAT_EQ(v.y, 0.F);
}

// operator+ combines corresponding components.
TEST(Vec2, AdditionCombinesComponents) {
    const ast::Vec2 a{1.F, 2.F};
    const ast::Vec2 b{3.F, 4.F};
    const ast::Vec2 result = a + b;
    EXPECT_FLOAT_EQ(result.x, 4.F);
    EXPECT_FLOAT_EQ(result.y, 6.F);
}

// operator* scales both components by the scalar.
TEST(Vec2, ScalarMultiplicationScalesBothComponents) {
    const ast::Vec2 v{2.F, 3.F};
    const ast::Vec2 result = v * 2.F;
    EXPECT_FLOAT_EQ(result.x, 4.F);
    EXPECT_FLOAT_EQ(result.y, 6.F);
}

// length() returns the Euclidean magnitude (3-4-5 triangle).
TEST(Vec2, LengthReturnsEuclideanMagnitude) {
    const ast::Vec2 v{3.F, 4.F};
    EXPECT_FLOAT_EQ(v.length(), 5.F);
}

// normalised() returns a unit vector (length 1).
TEST(Vec2, NormalisedReturnsUnitVector) {
    const ast::Vec2 v{3.F, 4.F};
    EXPECT_FLOAT_EQ(v.normalised().length(), 1.F);
}

// normalised() of the zero vector returns the zero vector without crashing.
TEST(Vec2, NormalisedZeroVectorReturnsZero) {
    const ast::Vec2 v{0.F, 0.F};
    const ast::Vec2 n = v.normalised();
    EXPECT_FLOAT_EQ(n.x, 0.F);
    EXPECT_FLOAT_EQ(n.y, 0.F);
}

// distance() returns the Euclidean distance between two points (3-4-5 triangle).
TEST(Vec2, DistanceReturnsEuclideanDistance) {
    const ast::Vec2 origin{0.F, 0.F};
    const ast::Vec2 point{3.F, 4.F};
    EXPECT_FLOAT_EQ(ast::Vec2::distance(origin, point), 5.F);
}

// MockRenderer can be constructed and used in EXPECT_CALL expressions.
TEST(MockRenderer, ExpectCallClearAndPresent) {
    ast::MockRenderer mock;
    EXPECT_CALL(mock, clear(testing::_)).Times(1);
    EXPECT_CALL(mock, present()).Times(1);
    mock.clear(ast::Colour{0, 0, 0, 255});
    mock.present();
}
