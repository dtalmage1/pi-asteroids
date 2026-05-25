#include <gtest/gtest.h>
#include "game/Game.hpp"
#include "game/ScoreTable.hpp"
#include "MockAudioSink.hpp"

namespace {

void startGame(ast::Game& game) {
    ast::InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

void tickFrames(ast::Game& game, int n) {
    const ast::InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
}

void killShip(ast::Game& game) {
    ast::Asteroid rock;
    rock.position   = game.ship().position;
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Large;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 999U);
    rock.active     = true;
    game.spawnAsteroid(rock);
    const ast::InputState noInput;
    game.update(1.0F / 60.0F, noInput);
}

// Exhaust all 3 lives and wait until the game-over timer puts us in GameOver.
void reachGameOver(ast::Game& game) {
    startGame(game);
    killShip(game);
    tickFrames(game, 1000);
}

} // namespace

// A fresh table has no entries.
TEST(ScoreTable, EmptyOnConstruct) {
    ast::ScoreTable t;
    EXPECT_EQ(t.count(), 0U);
}

// Submitting one score is stored and retrievable.
TEST(ScoreTable, SubmitOneScore) {
    ast::ScoreTable t;
    t.submit(500);
    ASSERT_EQ(t.count(), 1U);
    EXPECT_EQ(t.entry(0), 500);
}

// Scores are kept in descending order.
TEST(ScoreTable, SortedDescending) {
    ast::ScoreTable t;
    t.submit(100);
    t.submit(400);
    t.submit(250);
    ASSERT_EQ(t.count(), 3U);
    EXPECT_EQ(t.entry(0), 400);
    EXPECT_EQ(t.entry(1), 250);
    EXPECT_EQ(t.entry(2), 100);
}

// Table fills to kScoreTableSize.
TEST(ScoreTable, FillsToCapacity) {
    ast::ScoreTable t;
    for (int i = 1; i <= static_cast<int>(ast::kScoreTableSize); ++i) {
        t.submit(i * 100);
    }
    EXPECT_EQ(t.count(), ast::kScoreTableSize);
}

// A score lower than the worst entry in a full table is rejected.
TEST(ScoreTable, LowScoreRejectedWhenFull) {
    ast::ScoreTable t;
    for (int i = 0; i < static_cast<int>(ast::kScoreTableSize); ++i) {
        t.submit(1000);
    }
    t.submit(1);  // below every existing entry
    ASSERT_EQ(t.count(), ast::kScoreTableSize);
    EXPECT_EQ(t.entry(ast::kScoreTableSize - 1U), 1000);
}

// A high score displaces the worst entry in a full table.
TEST(ScoreTable, HighScoreDisplacesWorst) {
    ast::ScoreTable t;
    t.submit(100);
    t.submit(200);
    t.submit(300);
    t.submit(400);
    t.submit(500);
    t.submit(9999);
    ASSERT_EQ(t.count(), ast::kScoreTableSize);
    EXPECT_EQ(t.entry(0), 9999);
    EXPECT_EQ(t.entry(ast::kScoreTableSize - 1U), 200);
}

// Game submits score to the table when the GameOver timer expires.
TEST(ScoreTable, GameSubmitsScoreOnTimerExpiry) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);                    // → GameOver
    tickFrames(game, 310);                  // > 5 s → Attract, score submitted
    ASSERT_EQ(game.scoreTable().count(), 1U);
}

// Pressing START from GameOver also submits the score.
TEST(ScoreTable, GameSubmitsScoreOnStartFromGameOver) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    reachGameOver(game);
    ast::InputState startInput;
    startInput.start = true;
    game.update(1.0F / 60.0F, startInput);  // → Attract, score submitted
    ASSERT_EQ(game.scoreTable().count(), 1U);
}

// Scores from multiple games all appear in the table (up to capacity).
TEST(ScoreTable, MultipleGamesAccumulateScores) {
    ast::MockAudioSink audio;
    ast::Game game(audio, {800.0F, 600.0F});
    for (std::size_t i = 0U; i < ast::kScoreTableSize; ++i) {
        reachGameOver(game);
        tickFrames(game, 310);  // → Attract
    }
    EXPECT_EQ(game.scoreTable().count(), ast::kScoreTableSize);
}
