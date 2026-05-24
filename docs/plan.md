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

## M3 — Feature Backlog

**Goal:** Decompose the architecture and requirements into an ordered, estimated backlog of
implementable features. Each item is small enough to be completed in one iteration
(one branch, one PR).

**Backlog (initial — to be refined after M2):**

### Infrastructure
| ID | Feature | Notes |
|----|---------|-------|
| INF-1 | CMakeLists.txt: project skeleton, SDL2 via FetchContent | Host build only |
| INF-2 | GoogleTest + GoogleMock integration | `ctest` target |
| INF-3 | clang-tidy configuration (`.clang-tidy`) | Enforced in CI |
| INF-4 | GitHub Actions CI (build + lint + unit tests) | Ubuntu runner |
| INF-5 | Deploy & integration-test scripts | `scripts/deploy.sh` |
| INF-6 | RPi self-hosted runner or manual integration-test workflow | |

### Core Interfaces & Stubs
| ID | Feature | Notes |
|----|---------|-------|
| CORE-1 | `IRenderer` abstract interface + SDL2 implementation stub | No drawing yet |
| CORE-2 | `IInputSource` abstract interface + SDL2/joystick implementation | Button mapping |
| CORE-3 | Main game loop skeleton (fixed timestep, quit event) | Renders blank window |
| CORE-4 | `GameState` enum and state machine scaffold | No transitions yet |

### Game Entities
| ID | Feature | Notes |
|----|---------|-------|
| ENT-1 | `Ship` — position, orientation, wrapping | Physics only, no render |
| ENT-2 | `Ship` — thrust & rotation from input | Newtonian, drag |
| ENT-3 | `Asteroid` — position, velocity, size, wrapping | Three sizes |
| ENT-4 | `Projectile` — fire, travel, lifetime expiry | From ship nose |
| ENT-5 | Collision detection — projectile vs. asteroid | AABB or circle |
| ENT-6 | Asteroid splitting on hit | Spawn two smaller; destroy smallest |
| ENT-7 | Ship–asteroid collision | Lose a life |

### Rendering
| ID | Feature | Notes |
|----|---------|-------|
| RND-1 | Vector-style line renderer (SDL2 `DrawLine` wrapper) | Colour, thickness |
| RND-2 | Ship wireframe rendering | Rotated polygon |
| RND-3 | Asteroid wireframe rendering | Irregular polygon per size |
| RND-4 | Projectile rendering | Small dot / short line |
| RND-5 | HUD — score, lives remaining | Bitmap or line font |
| RND-6 | Explosion / particle effect | Short-lived line segments |

### Game Logic
| ID | Feature | Notes |
|----|---------|-------|
| GAME-1 | Scoring rules (per asteroid size) | Matches arcade |
| GAME-2 | Wave spawning — asteroid count per wave | Classic progression |
| GAME-3 | Lives system and respawn logic | Invincibility period |
| GAME-4 | Game-over state | Transition to attract |
| GAME-5 | Attract mode / title screen | Auto-play or static |
| GAME-6 | High-score table (session, non-persistent) | Top 5 |
| GAME-7 | Hyperspace (emergency warp) | Random reposition |

### Audio (optional for v1.0)
| ID | Feature | Notes |
|----|---------|-------|
| AUD-1 | `IAudioSink` abstract interface + SDL2_mixer stub | Gated on NFR |
| AUD-2 | Thrust, fire, explosion sound effects | WAV assets |
| AUD-3 | Background beat (tempo scales with asteroid count) | Classic mechanic |

### Polish & Hardening
| ID | Feature | Notes |
|----|---------|-------|
| POL-1 | Controller hot-plug (connect/disconnect gracefully) | Runtime detection |
| POL-2 | Graceful SDL2 init failure handling | Error message + exit |
| POL-3 | Configurable resolution / fullscreen toggle | Command-line flag |
| POL-4 | Frame-rate cap / VSync option | Prevent CPU thrash on RPi |

**Backlog ordering:** INF → CORE → ENT (physics before render) → RND → GAME → AUD → POL.
Audio items are deferred to post-v1.0 if they slip.

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

A backlog item is **Done** when all of the following are true:

**Code**
- [ ] Feature behaves as specified by the finalised acceptance criteria in `docs/requirements.md`
- [ ] All new code has corresponding unit tests (GoogleTest); coverage of new logic ≥ 80%
- [ ] Integration test exists or manual test procedure documented (for hardware-dependent items)
- [ ] `clang-tidy` reports zero warnings on new/modified files
- [ ] CI pipeline (build + lint + unit tests) is green on the PR
- [ ] No raw owning pointers introduced; RAII throughout
- [ ] No magic numbers; all constants are `constexpr` named values

**Documentation**
- [ ] `docs/requirements.md` — affected entries promoted from outline to final; any
  implementation-driven changes to scope or values recorded
- [ ] `docs/architecture.md` — updated to reflect any design decisions made or changed
  during implementation (new interfaces, revised ownership rules, etc.)
- [ ] Test descriptions — each GoogleTest `TEST` / `TEST_F` has a one-line comment stating
  what it verifies; integration test procedures updated in `tests/integration/`
- [ ] `CHANGELOG.md` entry drafted (merged by the PR process)

**Process**
- [ ] PR description references the backlog item ID and the finalised acceptance criteria
- [ ] Merged to `main` via PR (no direct commits)

---

## MR — Release v1.0

1. All backlog items through GAME-6 Done and merged to `main`
2. Integration tests passing on physical RPi 400 with PiHut controller
3. Version set: `constexpr int VERSION_MAJOR = 1, VERSION_MINOR = 0, VERSION_PATCH = 0`
4. `CHANGELOG.md` written (one entry per merged PR, auto-assembled from PR titles)
5. `docs/release.md` checklist completed
6. Git tag `v1.0.0` pushed → GitHub Actions publishes release artifact

---

## Document Status

| Document | Status |
|----------|--------|
| `docs/plan.md` | **Done** (this file) |
| `docs/architecture.md` | **M0 outline complete** — 7 open questions for M2 |
| `docs/requirements.md` | Not started (M1) |
| `docs/release.md` | Not started (MR) |
