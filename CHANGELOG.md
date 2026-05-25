# Changelog

All notable changes to this project are documented here.
Format: one entry per merged PR, newest first.

---

## [Unreleased]

### Added
- RND-4: Active projectile rendered as a short line segment (4px total, centred on
  `position`, oriented along normalised velocity) via `IRenderer::drawLine`;
  inactive projectiles skipped. 2 new tests; 67/67 passing.

### Added
- ENT-4: `ProjectileOwner` enum (`Player`/`Saucer`); `Projectile` struct (position,
  velocity, lifetime, owner, active); `kMaxProjectiles=4` constexpr; `Game::projectiles()`
  accessor; firing from nose tip (`ship.position + nose * 15px`) on `InputState::fire`
  when Playing and a free slot exists; projectile update loop decrements lifetime and
  deactivates expired shots; position integrated and wrapped each frame.
  8 new tests; 65/65 passing.

### Added
- ENT-3: `AsteroidSize` enum (`Large`/`Medium`/`Small`); `Asteroid` struct (position,
  velocity, angle, angularVel, size, shape, active; radii 48/24/12 px);
  `generateAsteroidShape(size, vertices, seed)` — seeded `std::minstd_rand`, ±20% jitter,
  local-space polygon; `Game::spawnAsteroid` / `Game::asteroids()` accessor;
  asteroid update loop (rotate + integrate + wrap) runs each frame.
  `Asteroid.cpp` added to `lib_game`. 11 new tests; 54/54 passing.

### Added
- ENT-2: `Game::update` handles `rotateLeft`/`rotateRight` (3.5 rad/s), `thrust`
  (200 px/s² via `applyThrust`), and `ship_.thrusting` flag; `Attract→Playing`
  transition on `InputState::start` (ship reset to centre, zero velocity, zero angle);
  rotation wrapped via `wrapAngle` each frame. 5 new tests; 43/43 passing.

### Added
- RND-2: `Game::render` draws ship chevron wireframe each frame via `IRenderer::drawLineStrip`
  (4 vertices: nose `(0,−15)`, right wing `(9,9)`, tail notch `(0,4)`, left wing `(−9,9)`,
  rotated by `ship_.angle` using 2-D rotation matrix, translated to `ship_.position`);
  ship spawns at screen centre; `buildShipVertices` helper in anonymous namespace;
  `GameTest.RenderDrawsShipWireframe` replaces `RenderDoesNotCrash`;
  `GameTest.ShipSpawnsAtScreenCenter` added. 38/38 tests passing.

### Added
- RND-1: `Sdl2Renderer::drawLine` implemented via `SDL_RenderDrawLineF`;
  `Sdl2Renderer::drawLineStrip` implemented with iterator traversal and `closed` support
  (extra segment back to first point); stubs replaced. 37/37 tests unaffected.

### Added
- ENT-1: `Physics.hpp` inline free functions (`integratePosition`, `applyThrust`,
  `applyDrag`, `wrapPosition`, `wrapAngle`); `Collision.hpp` (`circlesOverlap`);
  `Ship` struct (position, velocity, angle, invincTimer, thrusting, active; kRadius=10);
  `Game::update` applies ship drift (drag → integrate → wrap) in Playing state;
  `Game::ship()` const accessor. Architecture doc corrected: thrust vector is
  `Vec2(sin θ, −cos θ)` not `Vec2(−sin θ, −cos θ)`. 18 new tests; 37/37 passing.

### Added
- CORE-5: `Game` class; `GameState` enum (`Attract`, `Playing`, `PlayerDead`, `GameOver`);
  state machine scaffold; `Game::update(dt, InputState)` and `Game::render(IRenderer&)` stubs;
  `main.cpp` wired with `Sdl2InputSource`, `Sdl2AudioSink`, and `Game`; dt computed from
  `SDL_GetPerformanceCounter` difference, capped at 50 ms; 3 new unit tests (19/19 passing).

### Added
- CORE-4: `Platform` RAII class (`SDL_Init`/`SDL_Quit`, `SDL_Window*` via `unique_ptr`);
  60 Hz fixed-step game loop in `main.cpp`; `SDL_MAIN_HANDLED` prevents SDL redefining `main`
  on Windows/MSVC; dt tracked from `SDL_GetPerformanceCounter`; `SDL_QUIT` event handling.
  `SDL2::SDL2-static` added to `asteroids` executable (SDL2 include paths don't propagate
  through PRIVATE-linked libs). Verified: zero clang-tidy findings and 16/16 tests on both
  host (MSVC/Ninja) and RPi (GCC/ARM64/Makefile).

### Added
- CORE-3: `IAudioSink` pure interface (`play`, `loop`, `stop`, `isPlaying`) in `game/`
  (no SDL2 dependency); `NullAudioSink` header-only no-op; `Sdl2AudioSink` stub
  (`Mix_OpenAudio`/`Mix_CloseAudio` RAII, playback deferred to AUD-1); `MockAudioSink`
  GoogleMock implementation. `SDL2::SDL2-static` added to `lib_audio` link to resolve
  `SDL_stdinc.h` include path. 4 new unit tests (16/16 passing on both targets).

### Added
- CORE-2: `InputState` struct (7 bool fields, all default `false`); `IInputSource` pure
  interface (`query() const`) in `game/` (no SDL2 dependency); `Sdl2InputSource` stub opens
  joystick index 0 at startup; PiHut SNES button map hardcoded (D-pad + A/B/X/Start);
  `connected` always `true` (hot-plug deferred to POL-1); `MockInputSource` GoogleMock
  implementation. 3 new unit tests (12/12 passing on both targets).

### Added
- CORE-1: Foundation types (`Vec2`, `Colour`, `SoundId`) and `IRenderer` pure interface in
  `src/game/` (no SDL2 dependency); `Sdl2Renderer` stub in `src/rendering/` (`clear` +
  `present` functional, `drawLine`/`drawLineStrip` stubs for RND-1); `MockRenderer` via
  GoogleMock in `tests/unit/`; 8 Vec2 unit tests + 1 MockRenderer smoke test (9/9 passing).
  `.clang-tidy` extended with `-bugprone-easily-swappable-parameters` (mathematical APIs
  naturally pair same-type params). Verified: zero clang-tidy findings and ctest 9/9 on
  both host (MSVC/Ninja) and RPi (GCC/ARM64/Makefile).

### Added
- INF-5: `scripts/deploy.sh` (rsync to `dan@dtdan`, excludes `.git/` and `build/`);
  `scripts/run_integration_tests.sh` (SSH build + `ctest` on RPi). Both use
  `set -euo pipefail`; exit non-zero on failure. Verified: build and ctest pass
  on RPi 400 (GCC/ARM64). Waiver: RPi CI runner is post-v1.0 optional (INF-6).
- INF-4: GitHub Actions CI workflow (`.github/workflows/ci.yml`); triggers on push
  and PR to `main`; single `ubuntu-latest` job: install system deps, configure with
  Ninja + GCC (`CMAKE_BUILD_TYPE=Debug`), build (clang-tidy inline), `ctest`.
  SDL2 and SDL2_mixer via FetchContent — no preinstalled SDL2 required. CI DoD
  waiver lifted; all subsequent PRs require green CI before merge.
- INF-3: `.clang-tidy` with `bugprone-*`, `clang-analyzer-*`, `cppcoreguidelines-*`,
  `modernize-*`, `performance-*`, `readability-*` checks; `WarningsAsErrors: '*'`;
  header filter excludes `_deps/`. CMake wires `CXX_CLANG_TIDY` on project targets
  via Ninja generator (Windows) and Makefile generator (RPi); `CMAKE_EXPORT_COMPILE_COMMANDS`
  enabled for clangd/IDE integration. Verified: zero findings on host (MSVC/Ninja,
  clang-tidy 17.0.3) and RPi (GCC/Makefile, clang-tidy 19.1.7).
- INF-2: GoogleTest (v1.14.0) and GoogleMock via FetchContent; `unit_tests` executable
  linked against `gtest_main`, `gmock`, and `lib_game`; one placeholder passing test
  (`Placeholder.AlwaysPasses`); `gtest_discover_tests` wires tests into `ctest`.
  Verified: `ctest --output-on-failure` passes 1/1 on MSVC/Windows host; full build
  verified on RPi 400 (GCC/ARM64, Raspberry Pi OS Bookworm).
- INF-1: CMake project skeleton — SDL2 (release-2.30.9) and SDL2_mixer (release-2.8.0,
  WAV only) via FetchContent, both static; five library targets (`lib_game` with no SDL2
  dep, `lib_platform`, `lib_rendering`, `lib_input`, `lib_audio`); `asteroids`
  executable exits 0; `tests/` scaffold; `.gitignore`. SDL2_mixer target resolved via
  probe loop (`SDL2_mixer::SDL2_mixer-static`). Verified: MSVC/Windows host.
