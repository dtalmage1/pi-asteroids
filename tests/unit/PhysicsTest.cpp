#include <gtest/gtest.h>
#include "game/physics/Physics.hpp"

// integratePosition moves pos by vel * dt.
TEST(Physics, IntegratePositionMovesAlongVelocity) {
    const ast::Vec2 result = ast::integratePosition({0.0F, 0.0F}, {100.0F, -50.0F}, 0.1F);
    EXPECT_FLOAT_EQ(result.x, 10.0F);
    EXPECT_FLOAT_EQ(result.y, -5.0F);
}

// applyThrust with angle=0 adds velocity in the up direction (negative Y).
TEST(Physics, ApplyThrustAngleZeroAcceleratesUp) {
    const ast::Vec2 result = ast::applyThrust({0.0F, 0.0F}, 0.0F, 100.0F, 1.0F);
    EXPECT_NEAR(result.x, 0.0F, 1e-5F);
    EXPECT_NEAR(result.y, -100.0F, 1e-5F);
}

// applyThrust with angle=π/2 (right) adds velocity in the positive X direction.
TEST(Physics, ApplyThrustAngleHalfPiAcceleratesRight) {
    constexpr float kHalfPi = 1.57079632F;
    const ast::Vec2 result = ast::applyThrust({0.0F, 0.0F}, kHalfPi, 100.0F, 1.0F);
    EXPECT_NEAR(result.x, 100.0F, 1e-4F);
    EXPECT_NEAR(result.y, 0.0F, 1e-4F);
}

// applyDrag reduces speed without reversing direction.
TEST(Physics, ApplyDragReducesSpeed) {
    const ast::Vec2 result = ast::applyDrag({100.0F, 0.0F}, 1.0F, 1.0F / 60.0F);
    EXPECT_GT(result.x, 0.0F);
    EXPECT_LT(result.x, 100.0F);
}

// applyDrag with drag * dt > 1 clamps to zero, not negative.
TEST(Physics, ApplyDragClampedToZero) {
    const ast::Vec2 result = ast::applyDrag({100.0F, 0.0F}, 200.0F, 1.0F);
    EXPECT_FLOAT_EQ(result.x, 0.0F);
    EXPECT_FLOAT_EQ(result.y, 0.0F);
}

// wrapPosition keeps pos inside [0, screenSize).
TEST(Physics, WrapPositionRightEdge) {
    const ast::Vec2 result = ast::wrapPosition({820.0F, 10.0F}, {800.0F, 600.0F});
    EXPECT_NEAR(result.x, 20.0F, 1e-3F);
    EXPECT_FLOAT_EQ(result.y, 10.0F);
}

TEST(Physics, WrapPositionLeftEdge) {
    const ast::Vec2 result = ast::wrapPosition({-10.0F, 10.0F}, {800.0F, 600.0F});
    EXPECT_NEAR(result.x, 790.0F, 1e-3F);
}

TEST(Physics, WrapPositionTopEdge) {
    const ast::Vec2 result = ast::wrapPosition({10.0F, -5.0F}, {800.0F, 600.0F});
    EXPECT_NEAR(result.y, 595.0F, 1e-3F);
}

// wrapAngle keeps angle in [0, 2π).
TEST(Physics, WrapAngleLargerThanTwoPi) {
    constexpr float kTwoPi = 6.28318530F;
    const float result = ast::wrapAngle(kTwoPi + 1.0F);
    EXPECT_NEAR(result, 1.0F, 1e-5F);
}

TEST(Physics, WrapAngleNegative) {
    constexpr float kTwoPi = 6.28318530F;
    const float result = ast::wrapAngle(-0.1F);
    EXPECT_NEAR(result, kTwoPi - 0.1F, 1e-5F);
}

TEST(Physics, WrapAngleZeroUnchanged) {
    EXPECT_FLOAT_EQ(ast::wrapAngle(0.0F), 0.0F);
}
