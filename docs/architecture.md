# Asteroids — Architecture (Outline)

**Status:** M0 outline — open questions flagged for M2 resolution after requirements (M1).

---

## 1. System Context

```
┌─────────────────────────────────────────────────────┐
│                  Raspberry Pi 400                   │
│                                                     │
│  ┌──────────────┐     ┌───────────────────────────┐ │
│  │  asteroids   │────▶│  SDL2 (software renderer) │─┼──▶ HDMI display
│  │  (process)   │     └───────────────────────────┘ │
│  │              │     ┌───────────────────────────┐ │
│  │              │◀────│  SDL2 event queue         │◀┼─── /dev/input/js0
│  └──────────────┘     │  (joystick + window)      │ │    (PiHut gamepad)
│                       └───────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

The process is a single binary. SDL2 is the only external runtime dependency. There is no
network access, no file I/O during gameplay, and no GPU-accelerated rendering — the VideoCore
VI is not used; SDL2's software renderer is sufficient for vector-style line graphics.

---

## 2. Module Breakdown

```
Asteroids/
└── src/
    ├── main.cpp              Entry point — wires modules together, starts game loop
    ├── platform/             SDL2 lifecycle (init, window, teardown)
    ├── input/                IInputSource interface + SDL2 joystick implementation
    ├── rendering/            IRenderer interface + SDL2 line-drawing implementation
    └── game/                 All game logic — state machine, entities, physics, collision
```

### 2.1 `platform/`
Owns SDL2 initialisation and the window/surface. Provides typed wrappers around SDL2 RAII
handles (`SDL_Window`, `SDL_Renderer`). No game logic here.

### 2.2 `input/`
Defines `IInputSource` — the abstract interface the game loop queries each tick. The SDL2
implementation translates joystick button/axis events from SDL's event queue into a plain
`InputState` value struct. A mock implementation enables unit testing without hardware.

### 2.3 `rendering/`
Defines `IRenderer` — the abstract interface the game uses to draw. Operations are at game
primitive level (draw line, draw polygon, clear, present) rather than SDL2 level. The SDL2
implementation wraps `SDL_RenderDrawLine`. A mock records draw calls for assertion in tests.

### 2.4 `game/`
Contains everything that would be testable in isolation with mocked hardware:
- `Game` — top-level object; owns all entities and the state machine; drives update/render
- `StateMachine` — manages `GameState` transitions
- Entity types: `Ship`, `Asteroid`, `Projectile`, `Particle`
- Physics helpers (wrap-around, Newtonian integration)
- Collision detection
- Scoring and wave management

### 2.5 `main.cpp`
Constructs the concrete platform/input/renderer implementations, injects them into `Game`,
and runs the fixed-timestep loop. No game logic here — thin wiring only.

---

## 3. Key Interfaces

These are the boundaries that hardware-couple the game to the platform. Everything behind them
can be unit-tested with mocks.

### `IInputSource`
```
query() → InputState
```
`InputState` is a plain struct: booleans for each mapped action (thrust, rotateLeft,
rotateRight, fire, hyperspace) plus a connected flag. The game never sees raw joystick
button indices.

### `IRenderer`
```
clear()
drawLine(x1, y1, x2, y2, colour)
drawLines(points[], colour)        // for polygons
present()
screenSize() → (width, height)
```
Colour is a simple RGBA struct. The renderer does not know about game entities.

### `IAudioSink` *(outline — may be deferred to post-v1.0)*
```
play(SoundId)
setLooping(SoundId, bool)
stop(SoundId)
```
`SoundId` is a named enum. The game never references audio file paths.

---

## 4. Data Flow — Main Game Loop

```
┌─────────────┐
│  Game loop  │  fixed timestep (target: 60 Hz)
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│ 1. Poll SDL events  │  window close, joystick connect/disconnect
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ 2. IInputSource     │  query() → InputState
│    .query()         │
└──────┬──────────────┘
       │  InputState (thrust, rotate, fire, …)
       ▼
┌─────────────────────┐
│ 3. Game::update(    │  advance physics, check collisions,
│     dt, InputState) │  update state machine, spawn/destroy entities
└──────┬──────────────┘
       │  updated entity positions & game state
       ▼
┌─────────────────────┐
│ 4. Game::render(    │  walk entity list, call IRenderer primitives
│     IRenderer&)     │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ 5. IRenderer        │  SDL2 surface flip to display
│    .present()       │
└─────────────────────┘
```

`update` and `render` are separate passes. `update` is called on a fixed logical timestep;
rendering may be decoupled (render as fast as possible with interpolation) or locked to the
same rate. *Open question OQ-5.*

---

## 5. Game State Machine (Outline)

```
          ┌─────────┐
    ──▶   │ Attract │  title screen / demo; no player input consumed
          └────┬────┘
               │ Start button
               ▼
          ┌─────────┐
          │ Playing │◀─────────────────────────┐
          └────┬────┘                          │
               │ Ship hit, lives > 0           │ Respawn delay elapsed
               ▼                               │
          ┌────────────┐                       │
          │ PlayerDead │───────────────────────┘
          └────┬───────┘
               │ Lives == 0
               ▼
          ┌──────────┐
          │ GameOver │──▶ (after delay or button press) ──▶ Attract
          └──────────┘
```

State transitions are driven by game events (ship destroyed, wave cleared, etc.), not by
direct entity queries. The state machine holds no entity state — it signals `Game` to act.

---

## 6. Entity Model (Outline)

Entities are stored **by value** in typed collections, not as polymorphic heap objects.
Asteroids-scale entity counts (< 100 at any time) make this straightforward and avoids
virtual dispatch overhead and allocation fragmentation.

| Entity | Owner | Storage | Lifetime |
|--------|-------|---------|----------|
| `Ship` | `Game` | single instance | respawns in place; never destroyed while lives > 0 |
| `Asteroid` | `Game` | `std::vector<Asteroid>` | spawned per wave; destroyed on split/hit |
| `Projectile` | `Game` | `std::vector<Projectile>` | spawned on fire; destroyed on hit or timeout |
| `Particle` | `Game` | `std::vector<Particle>` | spawned on explosion; destroyed on lifetime expiry |

Entity update iterates each vector; dead entities are removed with erase-remove idiom after
the update pass to avoid iterator invalidation during iteration.

---

## 7. Coordinate System (Outline)

- **Origin:** top-left corner (SDL2 native — avoids per-frame Y-flip)
- **Units:** screen pixels (no separate world-space scale for this resolution)
- **Wrap-around:** when an entity's position exits `[0, W) × [0, H)`, it wraps to the
  opposite edge. Applied uniformly to all moving entities.
- **Rotation:** stored as a single `float` angle in radians, increasing clockwise (SDL2
  convention). Ship nose direction derived from this angle.

*Open question OQ-3: whether to use a centre-origin coordinate system internally and only
convert at render time. Needs resolution in M2.*

---

## 8. Threading Model

**Single-threaded.** One game loop thread handles event polling, update, and render in
sequence. Rationale:
- SDL2's event queue is not thread-safe without explicit locking
- Joystick input arrives via SDL events, not a separate blocking read
- Asteroids update logic has no parallelism to exploit
- Eliminates all synchronisation complexity; suitable for RPi 400's single-core throughput

No worker threads, no async I/O. If audio is added, SDL2_mixer manages its own internal
audio callback thread; the game calls it through `IAudioSink` only from the main thread.

---

## 9. Memory Ownership Strategy

- `main.cpp` owns concrete platform/input/renderer objects (stack or `unique_ptr`)
- `Game` receives non-owning references (`IInputSource&`, `IRenderer&`) — it never deletes them
- All entity collections inside `Game` use value semantics (`std::vector<T>`)
- No raw owning pointers anywhere; `new` / `delete` do not appear in game code
- SDL2 handle wrappers in `platform/` use custom deleters with `unique_ptr` to ensure cleanup

---

## 10. Build & Dependency Structure

```
executable: asteroids
    ├── lib: game        (no SDL2 dependency — pure logic)
    ├── lib: rendering   (depends on SDL2)
    ├── lib: input       (depends on SDL2)
    └── lib: platform    (depends on SDL2)

test executable: asteroids_tests
    ├── lib: game        (same game lib — no recompile)
    ├── mock: MockRenderer
    ├── mock: MockInputSource
    └── GoogleTest / GoogleMock
```

`lib: game` has **no SDL2 dependency**. This is the architectural guarantee that makes
unit testing tractable: all game logic compiles and runs without SDL2 present (as on the
GitHub Actions Ubuntu runner, or any dev machine without SDL2 installed).

---

## 11. Open Questions (to resolve in M2)

| ID | Question | Options | Impact |
|----|----------|---------|--------|
| OQ-1 | Asteroid shape: fixed polygon per size, or randomly generated within a radius band at spawn? | Fixed = deterministic/testable; random = more visual variety | Entity struct, rendering, tests |
| OQ-2 | Input mapping: hardcoded SNES button layout, or configurable via file? | Hardcoded is simpler for v1.0 | `IInputSource` impl, scope |
| OQ-3 | Coordinate origin: top-left throughout, or centre-screen internally with conversion at render? | Centre is more natural for physics; top-left is simpler | Physics, rendering, all entity code |
| OQ-4 | Font/HUD rendering: SDL2_ttf, bitmap sprite sheet, or line-drawn digits? | Line-drawn fits vector aesthetic; TTF adds a dependency | `IRenderer`, assets |
| OQ-5 | Update/render coupling: fixed-rate lock-step, or separate update timestep with render interpolation? | Lock-step is simpler; interpolation is smoother if RPi misses frames | Main loop, entity state |
| OQ-6 | Audio: integrate SDL2_mixer for v1.0, or defer entirely? | Adds dependency and assets; classic feel depends on audio | `IAudioSink`, CMakeLists, scope |
| OQ-7 | `GameState` machine: freestanding class or embedded in `Game`? | Freestanding is more testable; embedded is simpler | `game/` structure |

---

## Document Status

| Section | Completeness |
|---------|-------------|
| System context | Complete for M0 |
| Module breakdown | Complete for M0 |
| Key interfaces | Outline — method signatures to be finalised in M2 |
| Data flow | Complete for M0 |
| State machine | Outline — transition conditions to be finalised with requirements |
| Entity model | Outline — fields and invariants to be finalised in M2 |
| Coordinate system | Outline — OQ-3 unresolved |
| Threading model | Complete for M0 (decision made) |
| Memory ownership | Complete for M0 (decision made) |
| Build structure | Complete for M0 |
| Open questions | 7 flagged for M2 |
