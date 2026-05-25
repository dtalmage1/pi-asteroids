#include <gtest/gtest.h>
#include "game/physics/Collision.hpp"

// Circles whose centres are closer than the sum of radii overlap.
TEST(Collision, OverlappingCirclesDetected) {
    EXPECT_TRUE(ast::circlesOverlap({0.0F, 0.0F}, 10.0F, {15.0F, 0.0F}, 10.0F));
}

// Circles that are far apart do not overlap.
TEST(Collision, SeparatedCirclesNotDetected) {
    EXPECT_FALSE(ast::circlesOverlap({0.0F, 0.0F}, 5.0F, {100.0F, 0.0F}, 5.0F));
}

// Circles touching exactly (distance == sum of radii) are not considered overlapping.
TEST(Collision, ExactlyTouchingNotOverlapping) {
    EXPECT_FALSE(ast::circlesOverlap({0.0F, 0.0F}, 10.0F, {20.0F, 0.0F}, 10.0F));
}

// Concentric circles (same position) overlap regardless of radius.
TEST(Collision, ConcentricCirclesOverlap) {
    EXPECT_TRUE(ast::circlesOverlap({5.0F, 5.0F}, 1.0F, {5.0F, 5.0F}, 1.0F));
}
