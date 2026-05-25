#include <gtest/gtest.h>
#include <vector>
#include "game/entities/Projectile.hpp"
#include "game/Game.hpp"
#include "MockAudioSink.hpp"
#include "MockRenderer.hpp"

namespace {
void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}
} // namespace

TEST(Projectile, DefaultsToInactive) {
    const ast::Projectile p;
    EXPECT_FALSE(p.active);
}

TEST(Projectile, DefaultOwnerIsPlayer) {
    const ast::Projectile p;
    EXPECT_EQ(p.owner, ast::ProjectileOwner::Player);
}

// Pressing fire in Playing state creates an active projectile.
TEST(Game, FireSpawnsActiveProjectile) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::InputState input;
    input.fire = true;
    game.update(1.0F / 60.0F, input);

    bool anyActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { anyActive = true; break; }
    }
    EXPECT_TRUE(anyActive);
}

// No more than kMaxProjectiles projectiles can be active at once.
TEST(Game, FireCapAtFourProjectiles) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::InputState input;
    input.fire = true;
    for (int i = 0; i < 20; ++i) {
        game.update(1.0F / 60.0F, input);
    }

    int activeCount = 0;
    for (const auto& p : game.projectiles()) {
        if (p.active) { ++activeCount; }
    }
    EXPECT_LE(activeCount, static_cast<int>(ast::kMaxProjectiles));
}

// Active projectile integrates position each frame.
TEST(Game, ProjectileMovesEachFrame) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game); // ship angle=0, nose points up (-Y)

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    float yBefore = 0.0F;
    for (const auto& p : game.projectiles()) {
        if (p.active) { yBefore = p.position.y; break; }
    }

    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);

    float yAfter = yBefore;
    for (const auto& p : game.projectiles()) {
        if (p.active) { yAfter = p.position.y; break; }
    }
    EXPECT_LT(yAfter, yBefore); // moving upward (decreasing Y)
}

// Projectile deactivates once its lifetime runs out.
TEST(Game, ProjectileExpiresAfterLifetime) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    const ast::InputState noInput;
    for (int i = 0; i < 100; ++i) { // ~1.67s > 0.75s lifetime
        game.update(1.0F / 60.0F, noInput);
    }

    bool anyActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { anyActive = true; break; }
    }
    EXPECT_FALSE(anyActive);
}

// Fire input is ignored while the game is in Attract state.
TEST(Game, FireIgnoredInAttractState) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});

    ast::InputState input;
    input.fire = true;
    game.update(1.0F / 60.0F, input);

    bool anyActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { anyActive = true; break; }
    }
    EXPECT_FALSE(anyActive);
}

// Expired slot is reused when firing again.
TEST(Game, FireSlotReusedAfterExpiry) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    // Fill all 4 slots
    ast::InputState fireInput;
    fireInput.fire = true;
    for (int i = 0; i < 4; ++i) {
        game.update(1.0F / 60.0F, fireInput);
    }

    // Wait for all to expire
    const ast::InputState noInput;
    for (int i = 0; i < 100; ++i) {
        game.update(1.0F / 60.0F, noInput);
    }

    // Fire again — should succeed
    game.update(1.0F / 60.0F, fireInput);

    bool anyActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { anyActive = true; break; }
    }
    EXPECT_TRUE(anyActive);
}

// --- RND-4: projectile rendering ---

// Active projectile is drawn as a line segment.
TEST(Game, RenderDrawsActiveProjectile) {
    ast::MockAudioSink audio;
    ast::MockRenderer  renderer;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, true)).Times(1); // ship
    EXPECT_CALL(renderer, drawLine(testing::_, testing::_, testing::_)).Times(1); // projectile
    game.render(renderer);
}

// No drawLine call when no projectiles are active.
TEST(Game, RenderSkipsInactiveProjectiles) {
    ast::MockAudioSink audio;
    ast::MockRenderer  renderer;
    ast::Game game(audio, {800.0F, 600.0F});

    EXPECT_CALL(renderer, drawLineStrip(testing::_, testing::_, true)).Times(1); // ship only
    // drawLine must NOT be called — MockRenderer fails on unexpected calls
    game.render(renderer);
}

// --- ENT-5: projectile vs asteroid collision ---

// --- ENT-6: asteroid splitting ---

// Destroying a Large asteroid spawns 2 active Medium children.
TEST(Game, LargeAsteroidSplitsIntoTwoMedium) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::Asteroid rock;
    rock.position = {400.0F, 150.0F};
    rock.size     = ast::AsteroidSize::Large;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 1);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);
    const ast::InputState noInput;
    for (int i = 0; i < 25; ++i) { game.update(1.0F / 60.0F, noInput); }

    int mediumCount = 0;
    for (const auto& a : game.asteroids()) {
        if (a.active && a.size == ast::AsteroidSize::Medium) { ++mediumCount; }
    }
    EXPECT_EQ(mediumCount, 2);
}

// Destroying a Medium asteroid spawns 2 active Small children.
TEST(Game, MediumAsteroidSplitsIntoTwoSmall) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::Asteroid rock;
    rock.position = {400.0F, 150.0F};
    rock.size     = ast::AsteroidSize::Medium;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Medium, 10, 1);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);
    const ast::InputState noInput;
    for (int i = 0; i < 25; ++i) { game.update(1.0F / 60.0F, noInput); }

    int smallCount = 0;
    for (const auto& a : game.asteroids()) {
        if (a.active && a.size == ast::AsteroidSize::Small) { ++smallCount; }
    }
    EXPECT_EQ(smallCount, 2);
}

// Destroying a Small asteroid spawns no children.
TEST(Game, SmallAsteroidDoesNotSplit) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::Asteroid rock;
    rock.position = {400.0F, 150.0F};
    rock.size     = ast::AsteroidSize::Small;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Small, 10, 1);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);
    const ast::InputState noInput;
    for (int i = 0; i < 25; ++i) { game.update(1.0F / 60.0F, noInput); }

    int activeCount = 0;
    for (const auto& a : game.asteroids()) {
        if (a.active) { ++activeCount; }
    }
    EXPECT_EQ(activeCount, 0);
}

// Split children travel in different directions.
TEST(Game, SplitChildrenDiverge) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    ast::Asteroid rock;
    rock.position = {400.0F, 150.0F};
    rock.velocity = {0.0F, 50.0F}; // moving downward — children diverge left/right
    rock.size     = ast::AsteroidSize::Large;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 1);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);
    const ast::InputState noInput;
    for (int i = 0; i < 25; ++i) { game.update(1.0F / 60.0F, noInput); }

    std::vector<float> xVels;
    for (const auto& a : game.asteroids()) {
        if (a.active && a.size == ast::AsteroidSize::Medium) {
            xVels.push_back(a.velocity.x);
        }
    }
    ASSERT_EQ(xVels.size(), 2U);
    EXPECT_NE(xVels[0], xVels[1]);
}

// A projectile travelling into an asteroid deactivates both.
TEST(Game, ProjectileDestroysAsteroidOnHit) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game); // ship at (400, 300), angle=0 → nose pointing up (-Y)

    ast::Asteroid rock;
    rock.position  = {400.0F, 150.0F}; // directly above ship, in shot path
    rock.size      = ast::AsteroidSize::Large;
    rock.shape     = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 42);
    rock.active    = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    const ast::InputState noInput;
    for (int i = 0; i < 25; ++i) {
        game.update(1.0F / 60.0F, noInput);
    }

    bool projActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { projActive = true; break; }
    }
    EXPECT_FALSE(projActive);
    EXPECT_FALSE(game.asteroids().front().active);
}

// A projectile that misses a distant asteroid leaves both active.
TEST(Game, ProjectileDoesNotDestroyDistantAsteroid) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game); // ship at (400, 300), firing upward

    ast::Asteroid rock;
    rock.position  = {0.0F, 0.0F}; // far corner — not in shot path
    rock.size      = ast::AsteroidSize::Large;
    rock.shape     = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 42);
    rock.active    = true;
    game.spawnAsteroid(rock);

    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);

    const ast::InputState noInput;
    for (int i = 0; i < 5; ++i) {
        game.update(1.0F / 60.0F, noInput);
    }

    bool projActive = false;
    for (const auto& p : game.projectiles()) {
        if (p.active) { projActive = true; break; }
    }
    EXPECT_TRUE(projActive);
    EXPECT_TRUE(game.asteroids().front().active);
}
