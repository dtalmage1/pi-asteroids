# Asteroids — Development Plan

## Overview

This plan governs all development from first commit to v1.0 release. The process is
deliberately front-loaded on architecture: the system decomposition is designed and reviewed
before any production code is written. Requirements follow the same pattern at a coarse level —
an initial outline establishes the full scope and informs architecture — but each feature's
requirements are only hardened (acceptance criteria, edge cases, exact values) at the point
that feature is selected for development. Feature implementation then proceeds iteratively,
drawing from a curated backlog with a clear Definition of Done at each step.

---

## Milestone Sequence

| # | Milestone | Deliverable | Gate |
|---|-----------|-------------|------|
| M0 | Outline Architecture | `docs/architecture.md` (draft) | Review & sign-off |
| M1 | Outline Requirements | `docs/requirements.md` (draft) | Review & sign-off |
| M2 | Architecture Iteration | `docs/architecture.md` (final) | Consistent with requirements |
| M3 | Feature Backlog | `docs/plan.md` updated with backlog | Prioritised & estimated |
| M4 | Toolchain Bootstrap | Repo, CI, build, test harness compiling | CI green on empty project |
| M5–MN | Feature Iterations | One backlog item per iteration | Definition of Done below |
| MR | Release v1.0 | Tagged release artifact on RPi | Release checklist passed |

---

## M0 — Outline Architecture

**Goal:** Establish the high-level system decomposition before requirements constrain it.
Design from hardware up: what the platform gives us, how we layer on top.

**Scope of `docs/architecture.md` at this stage:**
- System context diagram (RPi 400 + SDL2 + joystick + display)
- Top-level module breakdown (game logic, rendering, input, platform, main loop)
- Data flow: input → game state update → render
- Key interface boundaries (what gets mocked in unit tests)
- Threading model (single-threaded game loop vs. separate input thread)
- Memory ownership strategy (RAII, no raw owning pointers)
- Open questions flagged for M2 resolution

**Exit gate:** Author (Dan) satisfied the decomposition reflects hardware constraints and
C++17 RAII idioms. No code written.

---

## M1 — Outline Requirements

**Goal:** Define what the game must do and how well it must do it before implementation
choices are locked in.

**Scope of `docs/requirements.md` at this stage:**
- **Functional requirements** — all gameplay areas identified and described at outline level
  (ship physics, asteroid splitting, projectiles, scoring, lives, wave progression, game
  states, attract mode); enough detail to inform architecture, not enough to lock down
  implementation
- **Non-functional requirements** — frame rate target, input latency budget, display
  resolution, audio (if any), startup time, controller hot-plug
- **Constraints** — RPi 400 CPU budget, SDL2 software renderer, single USB controller
- **Out of scope for v1.0** — explicitly listed to control scope creep

Each requirement entry is marked **outline** at this stage. When a feature is selected for
development (iteration selection step), the relevant requirements are promoted to **final**:
acceptance criteria are written, edge cases enumerated, and exact numeric values agreed.
`docs/requirements.md` is a living document throughout the project.

**Exit gate:** All gameplay areas are represented at outline level. Architecture draft
reviewed against requirements — any conflicts noted for M2.

---

## M2 — Architecture Iteration (Final)

**Goal:** Harden the architecture in light of the now-complete requirements. Resolve all
open questions flagged in M0.

**Changes expected at this stage:**
- Class/struct names agreed for each module
- Public interfaces defined (abstract base classes / pure virtual) for all hardware-touching
  code (`IRenderer`, `IInputSource`, `IAudioSink`, …)
- Game-state machine states and transitions enumerated
- Entity model: ship, asteroid (large/medium/small), projectile, particle — ownership and
  lifetime rules
- Coordinate system and physics units documented (pixels vs. world units, wrap-around rules)
- Sequence diagrams for the main game loop and controller hot-plug handling
- File and namespace layout finalised (matches `src/` tree in CLAUDE.md)

**Exit gate:** Architecture and requirements are mutually consistent. No design decision
left open. Ready to derive a feature backlog from this document.

---

## M3 — Feature Backlog (Final)

**Goal:** Ordered, dependency-aware backlog of implementable features. Each item fits in
one branch + one PR. Items are grouped into **vertical slices** — each slice produces
something runnable and verifiable on real hardware.

**WAV asset dependency:** AUD-2 and AUD-3 require sourced WAV files before they can be
selected. Identify and commit assets before those items reach the top of the queue.

---

### Slice 1 — Toolchain: build, test, lint, CI

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 1 | INF-1 | CMake skeleton: top-level `CMakeLists.txt`, `src/` and `tests/` targets, SDL2 + SDL2_mixer via FetchContent, `audio/` module stub | — | Host build; binary exits immediately |
| 2 | INF-2 | GoogleTest + GoogleMock via FetchContent; `ctest` target; one placeholder passing test | INF-1 | Verifies test harness wires up |
| 3 | INF-3 | `.clang-tidy` config; CMake `clang-tidy` integration; zero warnings on empty project | INF-1 | Enforced on all subsequent PRs |
| 4 | INF-4 | GitHub Actions CI: build + lint + unit tests on Ubuntu runner | INF-1..3 | Green CI gate required before any merge |
| 5 | INF-5 | `scripts/deploy.sh` (rsync to RPi) + `scripts/run_integration_tests.sh` | INF-1 | Manual workflow; RPi runner (INF-6) is post-v1.0 optional |

---

### Slice 2 — Core foundation: blank window on screen

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 6 | CORE-1 | Foundation types: `Vec2`, `Colour`, `SoundId` enum; `IRenderer` pure interface; `Sdl2Renderer` stub (clear + present, no drawing); `MockRenderer` (GoogleMock) | INF-1..4 | `lib: game` gets no SDL2 dep; all types live in `game/` |
| 7 | CORE-2 | `IInputSource` pure interface; `InputState` struct; `Sdl2InputSource` with hardcoded PiHut SNES button map; `MockInputSource` | CORE-1 | Button map constants; `connected` flag always true for now |
| 8 | CORE-3 | `IAudioSink` pure interface; `NullAudioSink` (no-op); `MockAudioSink` (GoogleMock); `Sdl2AudioSink` stub (init only, no playback yet) | CORE-1 | SDL2_mixer init failure → substitute `NullAudioSink` silently |
| 9 | CORE-4 | `Platform` RAII (SDL2 init/quit, window creation); `main.cpp` game loop: 60 Hz fixed timestep, SDL event poll, quit handling; renders blank window | CORE-1..3 | dt capped at 50 ms; `Sdl2Renderer::clear` + `present` called each tick |
| 10 | CORE-5 | `Game` class; `GameState` enum (`Attract`, `Playing`, `PlayerDead`, `GameOver`); state machine scaffold; `Game::update()` + `Game::render()` stubs | CORE-4 | Starts in `Attract`; no transitions yet |

---

### Slice 3 — Moving ship visible on screen

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 11 | ENT-1 | `Physics.hpp` free functions (integrate, drag, thrust, wrapPosition, wrapAngle); `Collision.hpp` circle-circle helper; `Ship` struct (position, velocity, angle, invincTimer, thrusting, active); ship update (no input — drifts) | CORE-5 | `lib: game` only; fully unit-testable |
| 12 | RND-1 | `Sdl2Renderer::drawLine` + `drawLineStrip` implemented (was stub); `Sdl2Renderer::screenSize()`; colour support | ENT-1 | First real pixels on screen |
| 13 | RND-2 | Ship wireframe rendered each frame (chevron polygon, rotated by `Ship::angle`) | ENT-1, RND-1 | Visible ship on screen |
| 14 | ENT-2 | Ship thrust, rotation, drag from `InputState`; `Attract→Playing` transition on START; ship steers and wraps | RND-2, CORE-2 | First interactive moment |

---

### Slice 4 — Shooting asteroids

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 15 | ENT-3 | `Asteroid` struct (position, velocity, angle, angularVel, size, shape polygon); random shape generation (seeded RNG); movement + wrap; three sizes | ENT-2 | Shape generated once at spawn; stored in struct |
| 16 | RND-3 | Asteroid wireframe rendered (stored shape polygon, rotated each frame) | ENT-3, RND-1 | Asteroids visible |
| 17 | ENT-4 | `Projectile` struct (position, velocity, lifetime, `ProjectileOwner`); fire from ship nose; movement + wrap; lifetime expiry; max 4 in-flight cap | ENT-3 | Player projectiles only (`ProjectileOwner::Player`) |
| 18 | RND-4 | Projectile rendered (short bright line segment) | ENT-4, RND-1 | Shots visible |
| 19 | ENT-5 | Collision: player `Projectile` vs `Asteroid` (circle-circle); destroy both on hit | ENT-4 | No scoring or splitting yet |
| 20 | ENT-6 | Asteroid splitting on hit: Large→2 Medium, Medium→2 Small, Small→nothing; child velocities diverge from parent | ENT-5 | Core gameplay mechanic |

---

### Slice 5 — It's a game

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 21 | ENT-7 | Ship–asteroid collision (circle-circle, ship not invincible); triggers `SHIP_DESTROYED` event in state machine | ENT-6 | Leads into lives system |
| 22 | RND-6 | `Particle` struct (position, velocity, lifetime, maxLifetime); explosion spawns particles on asteroid/ship destroy; particle rendering (short fading line segments) | ENT-7, RND-1 | Visual feedback for destruction |
| 23 | GAME-1 | Scoring: 20/50/100 pts per Large/Medium/Small asteroid; score tracked in `Game`; saucer scoring reserved for SAU-2/3 | ENT-6 | Score incremented in collision handler |
| 24 | GAME-2 | `Wave` class: spawn N large asteroids clear of ship; wave-clear detection; inter-wave delay; asteroid count progression to maximum | GAME-1 | `Playing` state stays active across waves |
| 25 | GAME-3 | Lives system (3 lives, decrement on `SHIP_DESTROYED`); respawn at centre after delay; `invincTimer` invincibility period; ship flashes while invincible; `PlayerDead` state | ENT-7, GAME-2 | Extra life at 10k pts also implemented here |
| 26 | RND-5 | HUD: line-drawn 7-segment-style score digits; ship-icon lives indicator | GAME-3, RND-1 | No SDL2_ttf; digits via `drawLine` |
| 27 | GAME-4 | `GameOver` state: display "GAME OVER", final score; transition to `Attract` on delay or START | GAME-3 | |
| 28 | GAME-5 | `Attract` state: title text, "PRESS START" prompt rendered via line-drawn digits/letters; static screen | GAME-4, RND-5 | No auto-play demo |
| 29 | GAME-6 | `ScoreTable`: top-5 session scores; updated on game over; displayed on `Attract` screen | GAME-5 | No persistence; resets on process restart |
| 30 | GAME-7 | Hyperspace: ship warps to random position; risk — random chance of instant destruction on arrival | GAME-3 | Classic risk mechanic; probability finalised at selection |

---

### Slice 6 — Saucer

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 31 | SAU-1 | `Saucer` entity (position, velocity, size, fireTimer, active); spawn timer in `Game`; enters from random edge, traverses screen, exits; no firing, no render yet | GAME-2 | At most one saucer at a time; `active` flag |
| 32 | SAU-4 | Saucer wireframe rendering (ellipse approximation or polygon); visible on screen | SAU-1, RND-1 | Render added immediately so saucer is visible |
| 33 | SAU-2 | Large saucer: random-direction fire; `Projectile` with `ProjectileOwner::Saucer`; player-projectile vs saucer collision; saucer scoring (200 pts) | SAU-4 | Saucer fire does not destroy asteroids |
| 34 | SAU-3 | Small saucer: aimed fire with accuracy spread (decreasing in later waves); ship vs saucer projectile collision; ship vs saucer body collision; scoring (1000 pts) | SAU-2 | Parameters (spread, fire rate) finalised at selection |

---

### Slice 7 — Audio

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 35 | AUD-1 | `Sdl2AudioSink` fully implemented: SDL2_mixer init, load WAV assets, `play`/`loop`/`stop`/`isPlaying`; channel management; `NullAudioSink` substituted on failure | CORE-3 | **Blocked until WAV assets are sourced and committed** |
| 36 | AUD-2 | Wire game events to audio cues: thrust loop (start/stop), fire click, small/large explosion; dispatched in `Game::update()` | AUD-1, GAME-3 | |
| 37 | AUD-3 | Saucer engine loop (start on spawn, stop on destroy/exit); saucer fire SFX | AUD-2, SAU-3 | |
| 38 | AUD-4 | Background beat: two alternating tones (`BeatLow`/`BeatHigh`); tempo (interval) inversely proportional to remaining asteroid count | AUD-3, GAME-2 | Classic tension mechanic |

---

### Slice 8 — Polish & hardening

| # | ID | Feature | Depends on | Notes |
|---|----|---------|------------|-------|
| 39 | POL-1 | Controller hot-plug: `SDL_JOYDEVICEADDED/REMOVED` handling in `Sdl2InputSource`; `connected` flag; game pauses input (ship coasts) while disconnected | CORE-2 | Can be pulled forward to CORE-2 if desired |
| 40 | POL-2 | Graceful SDL2 / SDL2_mixer init failure: error message to stderr, clean exit; `NullAudioSink` already covers audio failure | CORE-4, AUD-1 | |
| 41 | POL-3 | Command-line `--width` / `--height` / `--fullscreen` flags; all entity sizes scale to screen dimensions | CORE-4 | Architecture already requires scale-relative coordinates |
| 42 | POL-4 | Frame-rate monitoring: log average FPS to stderr in debug builds; vsync option via SDL `SDL_RENDERER_PRESENTVSYNC` flag | CORE-4 | Prevents CPU thrash on RPi |

---

## M4–MN — Feature Iterations

Each iteration follows this fixed process:

### Iteration Process

1. **Select** — pick the next backlog item (or a small cluster of tightly coupled items)
2. **Harden requirements** — promote the relevant entries in `docs/requirements.md` from
   outline to final: write acceptance criteria, enumerate edge cases, agree numeric values;
   update `docs/architecture.md` if design decisions are affected
3. **Branch** — `git checkout -b feature/<id>-<short-name>`
4. **Implement** — write production code + unit tests together; no untested code merged
5. **Lint** — `clang-tidy` passes locally before push
6. **CI** — all GitHub Actions stages green
7. **Review** — code review (self or pair); checklist below
8. **Update docs** — finalise all documentation changed by this feature (see DoD)
9. **Merge** — squash-merge to `main` via PR
10. **Update backlog** — mark item done; adjust estimates on remaining items if needed

### Definition of Done

A backlog item is **Done** when **all** of the following are true. There is no
"pending" state — an item is either Done or it is not. Partial completions are not
merged to `master`.

**Before implementation starts**
- [ ] Acceptance criteria written and agreed — added to `docs/requirements.md` (game
  features) or recorded in the PR description (infrastructure items with no game-facing
  behaviour); criteria must be specific enough to be checked, not just described

**Code**
- [ ] Every acceptance criterion has been manually verified to pass
- [ ] The feature builds without errors on at least one target (host or RPi). For
  build-system items, `cmake -B build && cmake --build build` must complete cleanly
- [ ] All new logic has corresponding unit tests (GoogleTest); coverage ≥ 80%
  — *legitimately waived only for items with no testable logic (e.g. pure CMake
  changes); waiver must be stated explicitly in the PR description*
- [ ] `clang-tidy` reports zero warnings on new/modified files
  — *legitimately waived until INF-3 is merged; state this explicitly*
- [ ] CI pipeline (build + lint + unit tests) is green
  — *legitimately waived until INF-4 is merged; state this explicitly*
- [ ] Integration test exists or manual test procedure documented
  (for any item that touches hardware)
- [ ] No raw owning pointers; RAII throughout
- [ ] No magic numbers; all constants are `constexpr` named values

**Documentation**
- [ ] `docs/requirements.md` — acceptance criteria entry exists (game features:
  promoted from outline to final; infrastructure: PR description suffices)
- [ ] `docs/architecture.md` — updated if any design decision was made or changed
  during implementation
- [ ] Test descriptions — each `TEST` / `TEST_F` has a one-line comment stating what
  it verifies; integration procedures updated in `tests/integration/` where applicable
- [ ] `CHANGELOG.md` entry added

**Process**
- [ ] All legitimate waivers (clang-tidy, CI, tests) explicitly stated in the PR/merge
  message with the reason, not silently omitted
- [ ] Merged to `master` via a feature branch (no direct commits to `master`)

---

## MR — Release v1.0

1. All backlog items INF-1 through POL-4 (items 1–42) Done and merged to `main`
2. Integration tests passing on physical RPi 400 with PiHut controller
3. Version set: `constexpr int VERSION_MAJOR = 1, VERSION_MINOR = 0, VERSION_PATCH = 0`
4. `CHANGELOG.md` written (one entry per merged PR, auto-assembled from PR titles)
5. `docs/release.md` checklist completed
6. Git tag `v1.0.0` pushed → GitHub Actions publishes release artifact

---

## Document Status

| Document | Status |
|----------|--------|
| `docs/plan.md` | **M3 final** — 42-item backlog, 8 vertical slices, dependencies explicit |
| Backlog progress | **INF-1** ✓ Done — build verified on host (MSVC, Windows). **INF-2** next. |
| `docs/architecture.md` | **M2 final** — all open questions resolved |
| `docs/requirements.md` | **M1 outline complete** — scope decisions confirmed, parameters TBD at feature selection |
| `docs/release.md` | Not started (MR) |
