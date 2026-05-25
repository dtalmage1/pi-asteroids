#include <gtest/gtest.h>
#include "game/entities/Ship.hpp"
#include "game/Game.hpp"
#include "MockAudioSink.hpp"

// Ship struct has the default values specified in the architecture.
TEST(Ship, DefaultValues) {
    const ast::Ship ship;
    EXPECT_FLOAT_EQ(ship.position.x, 0.0F);
    EXPECT_FLOAT_EQ(ship.position.y, 0.0F);
    EXPECT_FLOAT_EQ(ship.velocity.x, 0.0F);
    EXPECT_FLOAT_EQ(ship.velocity.y, 0.0F);
    EXPECT_FLOAT_EQ(ship.angle, 0.0F);
    EXPECT_FLOAT_EQ(ship.invincTimer, 0.0F);
    EXPECT_FALSE(ship.thrusting);
    EXPECT_TRUE(ship.active);
}

// Collision radius matches the architecture-specified value.
TEST(Ship, RadiusConstant) {
    EXPECT_FLOAT_EQ(ast::Ship::kRadius, 10.0F);
}

// Game exposes its ship via a const accessor.
TEST(Game, ShipAccessorReturnsActiveShip) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_TRUE(game.ship().active);
}
