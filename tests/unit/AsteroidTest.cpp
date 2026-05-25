#include <gtest/gtest.h>
#include "game/entities/Asteroid.hpp"
#include "game/Game.hpp"
#include "MockAudioSink.hpp"

// --- Asteroid struct ---

TEST(Asteroid, DefaultsToInactive) {
    const ast::Asteroid a;
    EXPECT_FALSE(a.active);
}

TEST(Asteroid, RadiusLarge) {
    EXPECT_FLOAT_EQ(ast::Asteroid::radius(ast::AsteroidSize::Large),
                    ast::Asteroid::kRadiusLarge);
}

TEST(Asteroid, RadiusMedium) {
    EXPECT_FLOAT_EQ(ast::Asteroid::radius(ast::AsteroidSize::Medium),
                    ast::Asteroid::kRadiusMedium);
}

TEST(Asteroid, RadiusSmall) {
    EXPECT_FLOAT_EQ(ast::Asteroid::radius(ast::AsteroidSize::Small),
                    ast::Asteroid::kRadiusSmall);
}

// --- generateAsteroidShape ---

TEST(Asteroid, GenerateShapeReturnsCorrectVertexCount) {
    const auto shape = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 42U);
    EXPECT_EQ(shape.size(), 10U);
}

TEST(Asteroid, GenerateShapeVerticesWithinExpectedRange) {
    constexpr int kVerts = 10;
    const auto shape = ast::generateAsteroidShape(ast::AsteroidSize::Large, kVerts, 42U);
    // Jitter is ±20% of base radius; 25% threshold gives float headroom.
    const float maxR = ast::Asteroid::kRadiusLarge * 1.25F;
    for (const auto& v : shape) {
        EXPECT_LE(v.length(), maxR);
    }
}

TEST(Asteroid, GenerateShapeDeterministicForSameSeed) {
    const auto s1 = ast::generateAsteroidShape(ast::AsteroidSize::Medium, 8, 99U);
    const auto s2 = ast::generateAsteroidShape(ast::AsteroidSize::Medium, 8, 99U);
    ASSERT_EQ(s1.size(), s2.size());
    for (std::size_t i = 0; i < s1.size(); ++i) {
        EXPECT_FLOAT_EQ(s1[i].x, s2[i].x);
        EXPECT_FLOAT_EQ(s1[i].y, s2[i].y);
    }
}

TEST(Asteroid, GenerateShapeDifferentForDifferentSeeds) {
    const auto s1 = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 1U);
    const auto s2 = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 12345U);
    bool anyDiff = false;
    for (std::size_t i = 0; i < s1.size(); ++i) {
        if (s1[i].x != s2[i].x || s1[i].y != s2[i].y) {
            anyDiff = true;
            break;
        }
    }
    EXPECT_TRUE(anyDiff);
}

// --- Game integration ---

TEST(Asteroid, ActiveAsteroidMovesEachFrame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::Asteroid rock;
    rock.position = {100.0F, 100.0F};
    rock.velocity = {60.0F, 0.0F};
    rock.active   = true;
    game.spawnAsteroid(rock);

    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);

    EXPECT_GT(game.asteroids().front().position.x, 100.0F);
}

TEST(Asteroid, ActiveAsteroidRotatesEachFrame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::Asteroid rock;
    rock.angularVel = 1.0F;
    rock.active     = true;
    game.spawnAsteroid(rock);

    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);

    EXPECT_GT(game.asteroids().front().angle, 0.0F);
}

TEST(Asteroid, InactiveAsteroidDoesNotMove) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::Asteroid rock;
    rock.position = {200.0F, 200.0F};
    rock.velocity = {100.0F, 0.0F};
    rock.active   = false;
    game.spawnAsteroid(rock);

    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);

    EXPECT_FLOAT_EQ(game.asteroids().front().position.x, 200.0F);
}
