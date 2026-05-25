# Asteroids — Requirements

**Status:** M1 outline — entries marked **[OUTLINE]** have not yet been hardened.
Entries are promoted to **[FINAL]** when the feature is selected for development (M4+).

---

## 1. Functional Requirements

### FR-1 Ship Movement

**[OUTLINE]**

- The ship has a position, a velocity, and an orientation (heading angle).
- The player can rotate the ship left and right; rotation is instantaneous per input, not
  velocity-based.
- The player can apply thrust in the direction the ship is currently facing; thrust
  accumulates velocity (Newtonian).
- Velocity decays over time (drag) so the ship slows to a stop when thrust is released.
- The ship wraps around screen edges (exits one side, appears on the opposite side).
- The ship's geometry is a simple wireframe triangle/chevron (classic arcade shape).

*Parameters to finalise at feature selection: rotation rate (deg/frame), thrust
acceleration, drag coefficient, max speed.*

---

### FR-2 Projectiles

**[OUTLINE]**

- Pressing fire launches a projectile from the tip of the ship in the ship's current
  facing direction.
- Projectiles travel in a straight line at fixed speed (no gravity).
- A projectile expires after a fixed time-to-live (disappears without effect if it does
  not hit anything).
- A maximum of N projectiles may be in flight simultaneously; further fire inputs are
  ignored while at the cap.
- Projectiles wrap around screen edges.

*Parameters to finalise: projectile speed, time-to-live, simultaneous cap.*

---

### FR-3 Asteroids

**[OUTLINE]**

- Asteroids exist in three sizes: large, medium, small.
- Each asteroid has a position, a velocity (constant — no acceleration), and a rotation
  (visual only, does not affect collision).
- Asteroid speed is randomised within a per-size range at spawn; direction is random.
- Asteroids wrap around screen edges.
- Asteroid geometry is an irregular convex polygon (same shape per size, or randomly
  generated — see OQ-1 in architecture).

*Parameters to finalise: speed ranges per size, polygon vertex counts/shapes.*

---

### FR-4 Collision Detection

**[OUTLINE]**

- Collision model: circular hitboxes. Each entity has a radius; a collision occurs when
  the distance between centres is less than the sum of radii.
- Pairs checked each frame: player projectile vs. asteroid, player projectile vs. saucer,
  saucer projectile vs. ship, ship vs. asteroid, ship vs. saucer.
- Ship vs. player-projectile collision is **not** checked (player cannot shoot themselves).
- Asteroid vs. asteroid collision is **not** checked (they pass through each other, as
  in the original).
- Saucer projectiles do **not** destroy asteroids (classic behaviour).

*Parameters to finalise: hitbox radii per entity size.*

---

### FR-5 Asteroid Splitting

**[OUTLINE]**

- When a projectile hits an asteroid:
  - **Large** asteroid → destroyed; spawns 2 medium asteroids
  - **Medium** asteroid → destroyed; spawns 2 small asteroids
  - **Small** asteroid → destroyed; no spawn
- The two child asteroids are spawned at the parent's position, with velocities in
  diverging directions (random angle offset from parent velocity, increased speed).
- The projectile is destroyed on impact.

*Parameters to finalise: child velocity magnitude, divergence angle range.*

---

### FR-6 Scoring

**[OUTLINE]**

- Points are awarded when a projectile destroys an asteroid, based on size:

  | Asteroid size | Points |
  |---------------|--------|
  | Large         | 20     |
  | Medium        | 50     |
  | Small         | 100    |

- Score is displayed on the HUD throughout gameplay.
- Score resets to zero at the start of each new game.

*Classic arcade values used as baseline; to confirm at feature selection.*

---

### FR-7 Lives

**[OUTLINE]**

- The player starts each game with 3 lives.
- One life is lost when the ship collides with an asteroid or a saucer projectile.
- Remaining lives are displayed on the HUD (as small ship icons or a count).
- When all lives are exhausted, the game transitions to Game Over.
- One extra life is awarded at 10,000 points (classic arcade threshold); no further
  bonus lives beyond that.

*Parameters to finalise at feature selection: maximum lives cap (to prevent exploit if
extra life is repeatable), HUD display style.*

---

### FR-8 Respawn

**[OUTLINE]**

- After losing a life (while lives remain), the ship respawns at the centre of the screen.
- On respawn the ship has zero velocity and a fixed initial orientation.
- A brief invincibility period follows respawn during which ship–asteroid collisions are
  ignored; the ship flashes visually to indicate this.
- Respawn is delayed if the centre of the screen is occupied by an asteroid (wait until
  clear, or respawn anyway — to decide at feature selection).

*Parameters to finalise: invincibility duration, respawn delay.*

---

### FR-9 Wave Progression

**[OUTLINE]**

- A **wave** begins by spawning a set of large asteroids. No medium or small asteroids
  spawn directly.
- A wave is complete when all asteroids (including all split children) are destroyed.
- The next wave begins immediately (or after a short delay) with more large asteroids.
- Asteroid count increases each wave up to a maximum.
- Asteroids may not spawn within a minimum distance of the ship's starting/respawn position.

*Parameters to finalise: initial asteroid count, increment per wave, maximum count,
spawn exclusion radius, inter-wave delay.*

---

### FR-10 Game States

**[OUTLINE]**

- **Attract** — displayed at startup and after Game Over; shows title, high-score table,
  and 'Press Start' prompt. Static screen (no auto-play demo).
- **Playing** — active game; all gameplay systems active.
- **PlayerDead** — brief pause after a life is lost; ship explosion animation plays;
  transitions to Respawn or Game Over.
- **GameOver** — displayed when all lives are lost; shows final score; transitions to
  Attract after a delay or on button press.

*Attract mode content (static vs. auto-play demo) to decide at feature selection.*

---

### FR-11 Hyperspace

**[OUTLINE]**

- The player can activate hyperspace at any time during play.
- The ship disappears and reappears at a random position on screen.
- Classic risk mechanic: there is a random chance the ship reappears inside an asteroid
  and is immediately destroyed (no invincibility on arrival).
- Included in v1.0.

*Parameters to finalise at feature selection: risk probability, whether a cooldown applies.*

---

### FR-12 High Score Table

**[OUTLINE]**

- The top 5 scores of the current session are recorded.
- Scores are displayed on the Game Over or Attract screen.
- Scores are **not** persisted to disk — the table resets on process restart (v1.0 scope).
- No name entry (initials) in v1.0 — scores only.

---

### FR-13 UFO / Flying Saucer Enemy

**[OUTLINE]**

- Two saucer sizes appear during play: large and small. Both are in scope for v1.0.
- Saucers spawn on a periodic random timer, independent of asteroid count (classic
  arcade behaviour). A saucer enters from one side of the screen, traverses to the
  other, then disappears (or is destroyed).
- The **large saucer** fires projectiles in random directions.
- The **small saucer** aims its projectiles at the player ship (increasing difficulty
  in later waves).
- Saucer projectiles can destroy the player ship (lose a life) but do not split
  asteroids.
- Scoring:

  | Saucer size | Points |
  |-------------|--------|
  | Large        | 200    |
  | Small        | 1,000  |

- Only one saucer is present at a time.

*Parameters to finalise at feature selection: spawn timer range, projectile fire rate,
small-saucer aim accuracy, saucer speed, wave at which small saucer begins appearing.*

---

## 2. Non-Functional Requirements

### NFR-1 Frame Rate

**[OUTLINE]**

- Target: 60 frames per second on RPi 400.
- Minimum acceptable: 30 fps sustained during heaviest gameplay (maximum asteroid count,
  particles active).
- The game loop uses a fixed logical timestep; rendering may drop frames without affecting
  physics determinism.

### NFR-2 Input Latency

**[OUTLINE]**

- Input-to-screen latency (button press → visible change): ≤ 2 frames at 60 Hz (≤ 33 ms).
- SDL2 joystick event polling happens once per game loop tick; no input buffering beyond
  one frame.

### NFR-3 Display Resolution

**[OUTLINE]**

- Default: 1920×1080 (full HD), fullscreen.
- All gameplay coordinates and entity sizes are expressed relative to screen dimensions so
  the game scales to other resolutions without code changes.
- A command-line flag may override resolution (e.g. `--width 1280 --height 720`).

### NFR-4 Startup Time

**[OUTLINE]**

- From process launch to Attract screen: ≤ 5 seconds on RPi 400.

### NFR-5 Controller Hot-Plug

**[OUTLINE]**

- If the controller is disconnected during play, the game pauses or ignores input
  gracefully (no crash).
- When the controller reconnects, gameplay resumes normally.
- If no controller is connected at startup, the game waits on the Attract screen
  (does not crash).

### NFR-6 Audio

**[OUTLINE]**

- Full audio in scope for v1.0, implemented via SDL2_mixer behind `IAudioSink`.
- Sound effects: thrust (looping), fire, small explosion (small asteroid), large
  explosion (medium/large asteroid or ship), saucer engine (looping while present),
  saucer fire.
- Background beat: two alternating low tones, tempo increasing as the asteroid count
  decreases (classic tension mechanic).
- The game is functional (silent) if SDL2_mixer initialisation fails; audio failure is
  non-fatal.

*Audio asset format and specific WAV files to identify at feature selection.*

---

## 3. Constraints

- **Platform:** Raspberry Pi 400 (ARM64, Raspberry Pi OS Bookworm 64-bit).
- **Renderer:** SDL2 software renderer only. No OpenGL, Vulkan, or VideoCore-specific APIs.
- **Input:** Single PiHut wireless USB SNES-style gamepad. No keyboard gameplay controls
  (keyboard may be used for developer/debug functions only).
- **Dependencies:** SDL2 (required). SDL2_mixer (required if audio in scope). No other
  external runtime dependencies.
- **Build:** CMake + C++17. Must build cleanly on Ubuntu (GitHub Actions runner) for CI.

---

## 4. Scope Decisions

All scope questions resolved during M1 review.

| ID | Question | Decision |
|----|----------|---------|
| SQ-1 | UFO/saucer enemy in v1.0? | **Yes — both sizes** (large random-fire, small aimed) |
| SQ-2 | Audio in v1.0? | **Yes — full audio** (SDL2_mixer, SFX + background beat) |
| SQ-3 | Hyperspace in v1.0? | **Yes — with classic risk mechanic** |
| SQ-4 | Attract mode: static title or auto-play demo? | **Static title screen** |
| SQ-5 | High-score name entry (initials) in v1.0? | **No** (scores only) |
| SQ-6 | Starting lives? | **3** |
| SQ-7 | Extra life threshold? | **10,000 points, once** |
| SQ-8 | Saucer spawn trigger? | **Periodic timer (classic)** |

---

## 5. Infrastructure Requirements

Infrastructure items have no game-facing behaviour. Their acceptance criteria are
recorded here rather than in PR descriptions so they are traceable.

### IR-1 CMake Build Skeleton **[FINAL]**

**Acceptance criteria — all must pass before INF-1 is Done:**

1. `cmake -B build` completes with exit code 0 and no errors (warnings are
   acceptable during this item; clang-tidy not yet configured)
2. `cmake --build build` completes with exit code 0
3. The `asteroids` binary exists in the build tree and exits with code 0 when run
4. Six CMake targets are defined and build successfully:
   `lib_game`, `lib_platform`, `lib_rendering`, `lib_input`, `lib_audio`, `asteroids`
5. `lib_game` has no SDL2 or SDL2_mixer link dependency (verified by inspecting
   CMake target properties or confirming it compiles on a machine without SDL2)
6. SDL2 is fetched at version `release-2.30.9`; SDL2_mixer at `release-2.8.0`,
   WAV support only (FLAC, MP3, OGG, OPUS, MIDI, MOD, WAVPACK all disabled)
7. The `tests/` subdirectory is present and `cmake --build build` does not error
   on it (GoogleTest not yet wired — stub only)

**Legitimate waivers for INF-1:**
- Unit tests: no testable logic exists (pure CMake + empty stubs) — waived
- clang-tidy: not configured until INF-3 — waived
- CI pipeline: not configured until INF-4 — waived
- RPi build verification: DoD required both targets from INF-2 onwards; INF-1 was
  verified on host only (MSVC/Windows). RPi build verified implicitly when INF-2
  passes criterion 2.

---

### IR-2 GoogleTest + GoogleMock Harness **[FINAL]**

**Acceptance criteria — all must pass before INF-2 is Done:**

1. `cmake -B build && cmake --build build` completes with exit code 0 on the host
   (MSVC/Windows) — no regression from INF-1
2. `cmake -B build && cmake --build build` completes with exit code 0 on the RPi
   (GCC/ARM64, Raspberry Pi OS Bookworm)
3. GoogleTest and GoogleMock are fetched via FetchContent at a pinned version tag
   (`v1.14.0`)
4. A `unit_tests` executable is defined; it links `gtest_main`, `gmock`, and `lib_game`
5. `ctest -N` lists at least one test (verified on both targets)
6. `ctest --output-on-failure` exits 0 with all tests passing (verified on both targets)
7. `lib_game` retains no SDL2 or SDL2_mixer link dependency (INF-1 not regressed)

**Legitimate waivers for INF-2:**
- clang-tidy: not configured until INF-3 — waived
- CI pipeline: not configured until INF-4 — waived

---

### IR-3 Clang-tidy Integration **[FINAL]**

**Acceptance criteria — all must pass before INF-3 is Done:**

1. `.clang-tidy` exists at project root with an agreed check set:
   `bugprone-*`, `clang-analyzer-*`, `cppcoreguidelines-*`, `modernize-*`,
   `performance-*`, `readability-*`; selected noisy checks disabled;
   `WarningsAsErrors: '*'`; `HeaderFilterRegex` excludes `_deps/` headers
2. clang-tidy wired into CMake via `CXX_CLANG_TIDY` on project targets only
   (not FetchContent deps); `CMAKE_EXPORT_COMPILE_COMMANDS ON` for IDE support
3. `cmake --build build` exits 0 on host (MSVC/Windows, Ninja generator) —
   zero clang-tidy findings on all project sources
4. `cmake --build build` exits 0 on RPi (GCC/ARM64, Makefile generator) —
   zero clang-tidy findings on all project sources
5. Clang-tidy DoD waiver (held since INF-1) is lifted; all subsequent items
   must maintain zero clang-tidy findings

**Legitimate waivers for INF-3:**
- CI pipeline: not configured until INF-4 — waived

---

### IR-4 GitHub Actions CI Pipeline **[FINAL]**

**Acceptance criteria — all must pass before INF-4 is Done:**

1. `.github/workflows/ci.yml` exists and triggers on push and PR to `main`
2. Single job on `ubuntu-latest`: install system deps → configure (Ninja + GCC,
   `CMAKE_BUILD_TYPE=Debug`) → build (clang-tidy runs inline via `CXX_CLANG_TIDY`)
   → `ctest --output-on-failure`
3. SDL2 and SDL2_mixer are fetched via FetchContent; no preinstalled SDL2 required
4. All stages pass with green status on push to `main`
5. CI DoD waiver (held since INF-1) is lifted; no merges to `main` without green CI

**Legitimate waivers for INF-4:**
- None

---

### IR-5 Deploy and Integration Test Scripts **[FINAL]**

**Acceptance criteria — all must pass before INF-5 is Done:**

1. `scripts/deploy.sh` exists; rsyncs project to `dan@dtdan` excluding `.git/`
   and `build/`; uses `set -euo pipefail`; works from any directory
2. `scripts/run_integration_tests.sh` exists; SSHs to RPi, runs
   `cmake --build build` then `ctest --output-on-failure`; exits non-zero on
   any failure; uses `set -euo pipefail`
3. Both scripts are executable (mode 755)
4. `run_integration_tests.sh` verified: build exits 0 and ctest passes on RPi

**Legitimate waivers for INF-5:**
- CI automation of RPi tests: RPi runner in CI is post-v1.0 (INF-6 optional)
- clang-tidy: passes (INF-3 done)

---

### IR-6 Foundation Types and IRenderer Interface **[FINAL]**

**Acceptance criteria — all must pass before CORE-1 is Done:**

1. `src/game/Vec2.hpp` defines `struct Vec2` in namespace `ast`:
   - Fields: `float x = 0.F`, `float y = 0.F`
   - `operator+(Vec2) const` → component-wise addition
   - `operator*(float) const` → scalar multiplication
   - `length() const` → Euclidean magnitude
   - `normalised() const` → unit vector; returns `{0,0}` when length is zero
   - `static distance(Vec2, Vec2) → float` → Euclidean distance
2. `src/game/Colour.hpp` defines `struct Colour` in namespace `ast`:
   - Fields: `uint8_t r, g, b, a` (default `a = 255`)
3. `src/game/SoundId.hpp` defines `enum class SoundId` in namespace `ast` with values:
   `Thrust, Fire, ExplosionSmall, ExplosionLarge, SaucerEngine, SaucerFire, BeatLow, BeatHigh`
4. `src/game/IRenderer.hpp` defines `class IRenderer` (pure interface) in namespace `ast`:
   - `virtual void clear(Colour) = 0`
   - `virtual void drawLine(Vec2, Vec2, Colour) = 0`
   - `virtual void drawLineStrip(const std::vector<Vec2>&, Colour, bool closed) = 0`
     (note: uses `std::vector` not `std::span` — C++17 compatibility)
   - `virtual void present() = 0`
   - `virtual Vec2 screenSize() const = 0`
   - Non-copyable, non-movable (deleted copy/move operators)
5. `src/rendering/Sdl2Renderer.hpp` / `.cpp` provide `class Sdl2Renderer : public IRenderer`:
   - `clear()` and `present()` are functional (call SDL2 renderer)
   - `drawLine()` and `drawLineStrip()` are no-op stubs (implemented in RND-1)
   - `screenSize()` queries SDL for the actual render output size
   - SDL_Renderer owned via `std::unique_ptr` with custom deleter (RAII)
   - `lib_game` retains no SDL2 dependency
6. `tests/unit/MockRenderer.hpp` provides `class MockRenderer : public IRenderer` using GoogleMock
7. Unit tests pass for `Vec2` (all 6 methods) and for `MockRenderer` basic usage
8. `cmake --build build` exits 0 on host (MSVC/Windows, Ninja) — zero clang-tidy findings
9. `cmake --build build` exits 0 on RPi (GCC/ARM64, Makefile) — zero clang-tidy findings
10. `ctest --output-on-failure` exits 0 on both host and RPi

---

### IR-7 IInputSource Interface and Sdl2InputSource Stub **[FINAL]**

**Acceptance criteria — all must pass before CORE-2 is Done:**

1. `src/game/InputState.hpp` defines `struct InputState` in namespace `ast`:
   - 7 bool fields, all defaulting to `false`:
     `thrust`, `rotateLeft`, `rotateRight`, `fire`, `hyperspace`, `start`, `connected`
2. `src/game/IInputSource.hpp` defines `class IInputSource` (pure interface) in namespace `ast`:
   - `virtual InputState query() const = 0`
   - Non-copyable, non-movable
3. `src/input/Sdl2InputSource.hpp` / `.cpp` provide `class Sdl2InputSource : public IInputSource`:
   - Constructor opens joystick index 0 if one is connected at startup; stores as `unique_ptr`
     with custom deleter (`SDL_JoystickClose`) — RAII
   - `query()` reads D-pad hat and 4 named button constants (B, A, X, Start) from the PiHut
     SNES layout; constants defined in `.cpp` anonymous namespace with `jstest` comment
   - `query()` always returns `connected = true` (hot-plug deferred to POL-1)
   - `query()` returns all-false action fields (connected still true) if no joystick is open
   - `lib_game` retains no SDL2 dependency
4. `tests/unit/MockInputSource.hpp` provides `class MockInputSource : public IInputSource`
   using GoogleMock
5. Unit tests cover: `InputState` default values; `MockInputSource::query()` returns
   configured state; mock can return different states on successive calls
6. `cmake --build build` exits 0 on host (MSVC/Ninja) — zero clang-tidy findings
7. `cmake --build build` exits 0 on RPi (GCC/ARM64/Makefile) — zero clang-tidy findings
8. `ctest --output-on-failure` exits 0 on both host and RPi

**Legitimate waivers for CORE-2:**
- Integration test of real button mapping: hardware-in-the-loop test not possible until
  CORE-4 wires the game loop; button indices documented for manual verification with
  `jstest /dev/input/js0`

---

### IR-8 IAudioSink Interface, NullAudioSink, and Sdl2AudioSink Stub **[FINAL]**

**Acceptance criteria — all must pass before CORE-3 is Done:**

1. `src/game/IAudioSink.hpp` defines `class IAudioSink` (pure interface) in namespace `ast`:
   - 4 pure virtual methods: `play(SoundId)`, `loop(SoundId)`, `stop(SoundId)`,
     `isPlaying(SoundId) const`
   - Non-copyable, non-movable
   - No SDL2 or SDL2_mixer dependency; includes only `game/SoundId.hpp`
2. `src/game/NullAudioSink.hpp` defines `class NullAudioSink : public IAudioSink`:
   - All methods are no-ops; `isPlaying()` always returns `false`
   - Header-only; no SDL2 or SDL2_mixer dependency
3. `src/audio/Sdl2AudioSink.hpp` / `.cpp` provide `class Sdl2AudioSink : public IAudioSink`:
   - Constructor calls `Mix_OpenAudio`; tracks success in `initialised_` bool
   - Destructor calls `Mix_CloseAudio()` if and only if `initialised_` is true — RAII
   - `play`, `loop`, `stop` are stubs (no-op); `isPlaying` returns `false` (audio wired
     in AUD-1 when WAV assets are available)
   - `lib_game` retains no SDL2 or SDL2_mixer dependency
4. `tests/unit/MockAudioSink.hpp` provides `class MockAudioSink : public IAudioSink`
   using GoogleMock
5. Unit tests cover: `NullAudioSink::isPlaying` returns `false` for any `SoundId`;
   `NullAudioSink` play/loop/stop methods callable without crash;
   `MockAudioSink` wires up correctly to the `IAudioSink` interface
6. `cmake --build build` exits 0 on host (MSVC/Ninja) — zero clang-tidy findings
7. `cmake --build build` exits 0 on RPi (GCC/ARM64/Makefile) — zero clang-tidy findings
8. `ctest --output-on-failure` exits 0 on both host and RPi

**Legitimate waivers for CORE-3:**
- No integration test for actual audio output — hardware-in-the-loop test deferred to
  AUD-1 when real WAV assets are sourced and `Sdl2AudioSink` playback is implemented

---

## 6. Out of Scope for v1.0

- Persistent high-score storage (file or database)
- Initials entry on high-score table
- Multiple control schemes or keyboard gameplay
- Network / multiplayer
- Two-player alternating (classic cabinet feature)
- Level editor or replay system
- Any platform other than RPi 400 / Raspberry Pi OS

---

## Document Status

| Section | Completeness |
|---------|-------------|
| FR-1 Ship movement | Outline |
| FR-2 Projectiles | Outline |
| FR-3 Asteroids | Outline |
| FR-4 Collision detection | Outline |
| FR-5 Asteroid splitting | Outline |
| FR-6 Scoring | Outline |
| FR-7 Lives | Outline |
| FR-8 Respawn | Outline |
| FR-9 Wave progression | Outline |
| FR-10 Game states | Outline |
| FR-11 Hyperspace | Outline |
| FR-12 High score table | Outline |
| FR-13 UFO/saucer | Outline — included in v1.0, parameters TBD |
| NFR-1 Frame rate | Outline |
| NFR-2 Input latency | Outline |
| NFR-3 Display resolution | Outline |
| NFR-4 Startup time | Outline |
| NFR-5 Controller hot-plug | Outline |
| NFR-6 Audio | Outline — included in v1.0, assets TBD |
| IR-1 CMake skeleton | **Final** |
| IR-2 GoogleTest harness | **Final** |
| IR-3 Clang-tidy integration | **Final** |
| IR-4 GitHub Actions CI | **Final** |
| IR-5 Deploy and integration test scripts | **Final** |
| IR-6 Foundation types and IRenderer interface | **Final** |
| IR-7 IInputSource interface and Sdl2InputSource stub | **Final** |
| IR-8 IAudioSink interface, NullAudioSink, and Sdl2AudioSink stub | **Final** |
