#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "MockAudioSink.hpp"

namespace {
void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}
} // namespace

// Score is zero immediately after construction.
TEST(Score, StartsAtZero) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    EXPECT_EQ(game.score(), 0);
}

// Destroying a Large asteroid awards 20 points.
TEST(Score, LargeAsteroidAwards20Points) {
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

    EXPECT_EQ(game.score(), 20);
}

// Destroying a Medium asteroid awards 50 points.
TEST(Score, MediumAsteroidAwards50Points) {
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

    EXPECT_EQ(game.score(), 50);
}

// Destroying a Small asteroid awards 100 points.
TEST(Score, SmallAsteroidAwards100Points) {
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

    EXPECT_EQ(game.score(), 100);
}

// Score accumulates across successive hits.
TEST(Score, ScoreAccumulates) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    startGame(game);

    // Two Small asteroids stacked in the upward shot path — one shot each
    ast::Asteroid rock1;
    rock1.position = {400.0F, 200.0F};
    rock1.size     = ast::AsteroidSize::Small;
    rock1.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Small, 10, 1);
    rock1.active   = true;
    game.spawnAsteroid(rock1);

    ast::Asteroid rock2;
    rock2.position = {400.0F, 120.0F};
    rock2.size     = ast::AsteroidSize::Small;
    rock2.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Small, 10, 2);
    rock2.active   = true;
    game.spawnAsteroid(rock2);

    // Fire twice on consecutive frames
    ast::InputState fireInput;
    fireInput.fire = true;
    game.update(1.0F / 60.0F, fireInput);
    game.update(1.0F / 60.0F, fireInput);

    const ast::InputState noInput;
    for (int i = 0; i < 40; ++i) { game.update(1.0F / 60.0F, noInput); }

    EXPECT_EQ(game.score(), 200);
}
