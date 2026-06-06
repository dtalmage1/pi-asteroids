#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "game/Game.hpp"
#include "game/entities/Asteroid.hpp"
#include "game/entities/Projectile.hpp"
#include "MockAudioSink.hpp"
#include "TestHelpers.hpp"

// Thrust input causes loop(Thrust) to be called each frame while held.
TEST(AudioEvent, ThrustLoopsSound) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    EXPECT_CALL(audio, loop(ast::SoundId::Thrust)).Times(testing::AtLeast(1));

    ast::InputState input;
    input.thrust = true;
    game.update(1.0F / 60.0F, input);
}

// Releasing thrust causes stop(Thrust) to be called.
TEST(AudioEvent, NoThrustStopsSound) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});

    EXPECT_CALL(audio, stop(ast::SoundId::Thrust)).Times(testing::AtLeast(1));

    ast::test::startGame(game);  // Playing state, thrust=false → stop(Thrust)
}

// Pressing fire causes play(Fire) exactly once per shot.
TEST(AudioEvent, FirePlaysSoundOnFire) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    EXPECT_CALL(audio, play(ast::SoundId::Fire)).Times(1);

    ast::InputState input;
    input.fire = true;
    game.update(1.0F / 60.0F, input);
}

// Destroying a Small asteroid plays ExplosionSmall.
TEST(AudioEvent, SmallAsteroidPlaysSmallExplosion) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    constexpr ast::Vec2 kPos{400.0F, 200.0F};

    ast::Asteroid rock;
    rock.position = kPos;
    rock.size     = ast::AsteroidSize::Small;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Small, 10, 1U);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::Projectile p;
    p.position = kPos;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Player;
    p.active    = true;
    game.spawnProjectile(p);

    EXPECT_CALL(audio, play(ast::SoundId::ExplosionSmall)).Times(1);

    ast::test::tickFrames(game, 1);
}

// Destroying a Medium asteroid plays ExplosionMedium.
TEST(AudioEvent, MediumAsteroidPlaysMediumExplosion) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    constexpr ast::Vec2 kPos{400.0F, 200.0F};

    ast::Asteroid rock;
    rock.position = kPos;
    rock.size     = ast::AsteroidSize::Medium;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Medium, 10, 1U);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::Projectile p;
    p.position = kPos;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Player;
    p.active    = true;
    game.spawnProjectile(p);

    EXPECT_CALL(audio, play(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::ExplosionMedium)).Times(1);

    ast::test::tickFrames(game, 1);
}

// Destroying a Large asteroid plays ExplosionLarge.
TEST(AudioEvent, LargeAsteroidPlaysLargeExplosion) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    constexpr ast::Vec2 kPos{400.0F, 200.0F};

    ast::Asteroid rock;
    rock.position = kPos;
    rock.size     = ast::AsteroidSize::Large;
    rock.shape    = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 1U);
    rock.active   = true;
    game.spawnAsteroid(rock);

    ast::Projectile p;
    p.position = kPos;
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Player;
    p.active    = true;
    game.spawnProjectile(p);

    EXPECT_CALL(audio, play(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::ExplosionLarge)).Times(1);

    ast::test::tickFrames(game, 1);
}

// Destroying a saucer plays ExplosionLarge.
TEST(AudioEvent, SaucerHitPlaysLargeExplosion) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);
    game.activateSaucer(ast::SaucerSize::Large, {400.0F, 150.0F});

    ast::Projectile p;
    p.position = {400.0F, 150.0F};
    p.velocity  = {0.0F, 0.0F};
    p.lifetime  = 1.0F;
    p.owner     = ast::ProjectileOwner::Player;
    p.active    = true;
    game.spawnProjectile(p);

    EXPECT_CALL(audio, play(ast::SoundId::ExplosionLarge)).Times(testing::AtLeast(1));

    ast::test::tickFrames(game, 1);
}

// Ship death stops the thrust loop and plays ExplosionLarge.
TEST(AudioEvent, ShipDeathStopsThrustAndPlaysExplosion) {
    testing::NiceMock<ast::MockAudioSink> audio;
    ast::Game game(audio, {800.0F, 600.0F});
    ast::test::startGame(game);

    EXPECT_CALL(audio, stop(ast::SoundId::Thrust)).Times(testing::AtLeast(1));
    EXPECT_CALL(audio, stop(ast::SoundId::SaucerEngine)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, stop(ast::SoundId::SaucerEngineSmall)).Times(testing::AnyNumber());
    EXPECT_CALL(audio, play(ast::SoundId::ExplosionLarge)).Times(1);

    ast::Asteroid rock;
    rock.position   = game.ship().position;
    rock.velocity   = {0.0F, 0.0F};
    rock.angularVel = 0.0F;
    rock.size       = ast::AsteroidSize::Large;
    rock.shape      = ast::generateAsteroidShape(ast::AsteroidSize::Large, 10, 999U);
    rock.active     = true;
    game.spawnAsteroid(rock);
    ast::test::tickFrames(game, 1);
}
