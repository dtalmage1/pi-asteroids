# Asteroids — Pre-Release Checklist

All items below must be resolved before the v1.0 release tag is created.
Items are grouped by type. Each has a status column: **Open**, **Done**, or **Won't Fix**
(with rationale).

---

## 1. Documentation Consistency

These are divergences found between the implementation and the architecture / requirements
documents discovered during a systematic pre-release review.

### 1.1 Fixes required before release

| # | Document | Issue | Action |
|---|----------|-------|--------|
| DC-1 | `architecture.md` §3 `IAudioSink` | `SoundId` enum shows 8 values; implementation has 10 (`ExplosionMedium` and `SaucerEngineSmall` added in AUD-audio). Comment "ExplosionLarge — ship or large/medium asteroid" is now wrong. | Update enum block and comment. |
| DC-2 | `architecture.md` §12 Memory ownership | Row for `Game` says "Receives `IRenderer&`, `IInputSource&`, `IAudioSink&`". Actual constructor signature is `Game(IAudioSink&, Vec2 screenSize)`. `IRenderer` is passed to `render()`, not the constructor; `IInputSource` is not a `Game` dependency at all (it is consumed in `main`). | Correct the row. |
| DC-3 | `architecture.md` §3 `IRenderer` | `drawLineStrip` signature shown as `std::span<const Vec2>`. Actual interface uses `const std::vector<Vec2>&`. (Requirements IR-6 already has the correct note.) | Update the code snippet in §3 to use `std::vector`. |
| DC-4 | `requirements.md` FR-7 / SQ-7 | Says "One extra life awarded at 10,000 points, **once only**". Implementation awards an extra life every 10,000 points (`nextExtraLifeScore_ += kExtraLifeScore`), which matches classic arcade behaviour. The requirement is wrong. | Change FR-7 to say "every 10,000 points" and remove the "once" constraint. Update SQ-7 accordingly. |
| DC-5 | `requirements.md` FR-8 Respawn | Says "to decide at feature selection" whether respawn is delayed for centre occupancy. Decision was never formally recorded. Implementation: always respawn after timer, with invincibility (no occupancy check). | Record the decision in FR-8: "respawn at centre regardless, invincibility timer compensates". |
| DC-6 | `requirements.md` FR-12 Score table | Says "displayed on the **Game Over or Attract** screen". Implementation: table appears on Attract only; Game Over shows score + "GAME OVER" text but not the full top-5 table. | Either implement table on Game Over (preferred — small change), or update FR-12 to say "Attract screen only" with rationale. |
| DC-7 | `architecture.md` §3 OQ-6 / Audio fallback | Says "a `NullAudioSink` is substituted silently". Actual fallback: `Sdl2AudioSink` uses an `initialised_` flag and becomes a no-op internally. `NullAudioSink` is never used at runtime. | Update §3 OQ-6 description to describe the `initialised_` flag approach. |

### 1.2 Minor / informational

| # | Document | Issue | Action |
|---|----------|-------|--------|
| DC-8 | `architecture.md` §8 Hot-plug | Says `Game::update()` checks `input.connected` and "ignores all action inputs". There is no explicit check; coasting is a natural side effect of the joystick returning all-false. Functionally equivalent. | Rephrase to describe the actual mechanism. |
| DC-9 | `architecture.md` §9 Saucer AI | Says "optional vertical weave for small saucer — TBD". SAU-3 was implemented without weave. TBD is now resolved. | Add a resolution note: "weave not implemented in v1.0; straight horizontal traversal only". |
| DC-10 | `requirements.md` NFR-6 Audio | Lists sound effects but misses `ExplosionMedium`, `SaucerEngineSmall`. | Extend the sound-effects list to match `SoundId.hpp`. |

---

## 2. Refactoring

### REF-1 Test helper duplication (recommended)

**Status: Open**

`startGame()` and `tickFrames()` are defined identically (or near-identically) in 11 of the
unit test files. They are not shared via a header. Every new test file that needs them must
copy them in.

**Impact:** not a correctness problem today, but the duplication makes the intent less clear
(which version is the reference?) and creates maintenance overhead if the helpers ever need
to change.

**Proposed action:**

1. Create `tests/unit/TestHelpers.hpp` (included by all tests that need it):

```cpp
#pragma once
#include "game/Game.hpp"

namespace ast::test {

inline void startGame(Game& game) {
    InputState input;
    input.start = true;
    game.update(1.0F / 60.0F, input);
}

inline void tickFrames(Game& game, int n) {
    const InputState noInput;
    for (int i = 0; i < n; ++i) { game.update(1.0F / 60.0F, noInput); }
}

} // namespace ast::test
```

2. Replace the per-file definitions in all 11 affected test files with a single include.

This is a safe mechanical refactor with no logic changes. Recommended to do before v1.0
so it is part of the shipped codebase.

**Files affected (11):**
`AttractTest.cpp`, `AudioEventTest.cpp`, `BeatTest.cpp`, `GameOverTest.cpp`,
`GameTest.cpp`, `HotplugTest.cpp`, `HudTest.cpp`, `HyperspaceTest.cpp`,
`LifeTest.cpp`, `SaucerFireTest.cpp`, `WaveTest.cpp`

---

## 3. Game Testing (RPi)

Manual play-test on physical hardware with the PiHut controller. All must pass before
release. Record pass/fail and any issues found.

### 3.1 Deployment

- [ ] `rsync` from host to RPi completes cleanly (`scripts/deploy.sh`)
- [ ] `cmake -B build && cmake --build build` on RPi exits 0
- [ ] `ctest --output-on-failure` on RPi: all tests green

### 3.2 Attract screen

- [ ] Attract screen displays on launch (black background, "ASTEROIDS" title, "PRESS START"
      prompt, score table)
- [ ] No crash if controller is disconnected at startup (Attract screen waits silently)
- [ ] Controller reconnect detected; START press accepted after reconnect

### 3.3 Starting and playing

- [ ] START transitions from Attract to Playing; ship appears at screen centre
- [ ] Ship rotates left and right with D-pad
- [ ] Thrust with D-pad up; ship accelerates in facing direction; coasts when released
- [ ] Asteroids spawn from screen edges; move and wrap correctly
- [ ] Projectile fired with A button; travels in correct direction; 4-shot cap enforced
- [ ] Asteroids split correctly: large → 2 medium, medium → 2 small, small → nothing
- [ ] Score increments correctly: 20 / 50 / 100 for large / medium / small
- [ ] HUD shows correct score (top-left) and lives (top-right ship icons)

### 3.4 Lives, respawn, and game over

- [ ] Ship explosion particles play on collision
- [ ] Lives decrement by 1 per death; HUD updates immediately
- [ ] Respawn after ~3 s with invincibility; ship flashes during invincibility
- [ ] Extra life awarded at 10,000 points (and 20,000, etc.); HUD updates
- [ ] Game Over after 3 lives lost; "GAME OVER" and final score shown
- [ ] Game Over auto-transitions to Attract after ~5 s
- [ ] START on Game Over returns to Attract immediately
- [ ] Score saved to table; appears on next Attract screen

### 3.5 Hyperspace

- [ ] X button activates hyperspace; ship vanishes and reappears at random position
- [ ] Occasionally (≈1-in-5) ship is destroyed on arrival; correct life lost

### 3.6 Saucer

- [ ] Large saucer appears after ~15 s; fires in random directions; exits opposite edge
- [ ] Small saucer appears; fires toward player; more dangerous than large
- [ ] Saucer engine sound starts/stops correctly on appear/destroy/exit
- [ ] Destroying a saucer awards 200 / 1,000 pts for large / small
- [ ] Saucer projectile kills ship; correct life lost

### 3.7 Wave progression

- [ ] Wave clears when all asteroids gone; new wave spawns after ~2 s
- [ ] Each wave spawns more large asteroids than the previous
- [ ] Asteroids never spawn on top of the ship (edge-spawning with exclusion zone)

### 3.8 Audio

- [ ] Thrust SFX loops while thrust held; stops on release
- [ ] Fire SFX plays each shot
- [ ] Small/medium/large explosion SFX sound distinct
- [ ] Background beat plays during waves; tempo increases as asteroid count decreases
- [ ] Beat stops between waves (no asteroids active)
- [ ] Saucer engine SFX loops while saucer active
- [ ] Saucer fire SFX plays on each saucer shot

### 3.9 Non-functional

- [ ] Frame rate subjectively smooth at 60 Hz during maximum-density wave
- [ ] No visible tearing (vsync or frame cap behaving correctly)
- [ ] Startup to Attract screen ≤ 5 s (NFR-4)
- [ ] No crash after 10+ minutes of continuous play
- [ ] Controller disconnect during play: ship coasts, no crash; reconnect resumes input

---

## 4. README

Write `README.md` at project root. Minimum content:

- [ ] Project description (one paragraph: faithful Asteroids clone on Raspberry Pi 400)
- [ ] Screenshot or video link (optional but recommended)
- [ ] Build prerequisites (CMake, MSVC / GCC, Ninja; SDL2 + SDL2_mixer fetched automatically)
- [ ] Build instructions (host Windows + RPi; copy the commands from `CLAUDE.md`)
- [ ] Run instructions (`./build/asteroids`, optional `--width`, `--height`, `--fullscreen` flags)
- [ ] Controls table (D-pad: rotate/thrust; A: fire; X: hyperspace; Start: start game)
- [ ] Running tests (`ctest --test-dir build --output-on-failure`)
- [ ] Licence note (if applicable)

---

## 5. Release Steps

After all items above are resolved:

1. Resolve all DC-* documentation issues (section 1.1)
2. Complete REF-1 refactor (section 2) — recommended before tagging
3. All game tests passing on RPi (section 3)
4. README written and committed (section 4)
5. Bump version constants in source and `CMakeLists.txt`:
   `VERSION_MAJOR=1`, `VERSION_MINOR=0`, `VERSION_PATCH=0`
6. Write `CHANGELOG.md` (one entry per PR merged to `main`)
7. CI green on `main`
8. Create and push tag: `git tag v1.0.0 && git push origin v1.0.0`
9. Verify GitHub Actions release artifact is published
10. Smoke-test the release artifact on RPi
