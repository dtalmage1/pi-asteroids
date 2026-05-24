# Asteroids — Architecture (Final)

**Status:** M2 final — all open questions resolved; consistent with M1 requirements.

---

## 1. System Context

```
┌──────────────────────────────────────────────────────────┐
│                    Raspberry Pi 400                      │
│                                                          │
│  ┌───────────────┐   ┌──────────────────────────────┐   │
│  │   asteroids   │──▶│ SDL2 (software renderer)     │───┼──▶ HDMI display
│  │   (process)   │   └──────────────────────────────┘   │
│  │               │   ┌──────────────────────────────┐   │
│  │               │◀──│ SDL2 event queue             │◀──┼─── /dev/input/js0
│  │               │   │ (joystick + window events)   │   │    (PiHut gamepad)
│  │               │   └──────────────────────────────┘   │
│  │               │   ┌──────────────────────────────┐   │
│  │               │──▶│ SDL2_mixer (audio callback)  │───┼──▶ audio out
│  └───────────────┘   └──────────────────────────────┘   │
└──────────────────────────────────────────────────────────┘
```

Single binary; no network, no file I/O during gameplay. SDL2 (rendering + input) and
SDL2_mixer (audio) are the only external runtime dependencies.

---

## 2. Module Breakdown & File Layout

```
src/
├── main.cpp                      Entry point; wires all modules, runs game loop
│
├── platform/
│   ├── Platform.hpp              SDL2 init/quit RAII wrapper; window creation
│   └── Platform.cpp
│
├── input/
│   ├── Sdl2InputSource.hpp       Concrete IInputSource; translates SDL joystick events
│   └── Sdl2InputSource.cpp
│
├── rendering/
│   ├── Sdl2Renderer.hpp          Concrete IRenderer; wraps SDL_RenderDrawLine etc.
│   └── Sdl2Renderer.cpp
│
├── audio/
│   ├── Sdl2AudioSink.hpp         Concrete IAudioSink; wraps SDL2_mixer
│   └── Sdl2AudioSink.cpp
│
└── game/                         No SDL2 or SDL2_mixer dependency in this subtree
    ├── Game.hpp / Game.cpp       Top-level; owns all entities and drives update/render
    │
    ├── IInputSource.hpp          Pure interface — defined here, consumed here
    ├── IRenderer.hpp             Pure interface — defined here, consumed here
    ├── IAudioSink.hpp            Pure interface — defined here, consumed here
    │
    ├── InputState.hpp            Plain struct; one bool per mapped action
    ├── Colour.hpp                Plain struct; r,g,b,a uint8_t
    ├── SoundId.hpp               enum class SoundId { ... }
    ├── Vec2.hpp                  float x,y; arithmetic operators
    │
    ├── entities/
    │   ├── Ship.hpp              Position, velocity, angle, state
    │   ├── Asteroid.hpp          Position, velocity, size, shape polygon
    │   ├── Saucer.hpp            Position, velocity, size, fire timer
    │   ├── Projectile.hpp        Position, velocity, lifetime, owner tag
    │   └── Particle.hpp         Position, velocity, lifetime (explosion debris)
    │
    ├── physics/
    │   ├── Physics.hpp           Integration, drag, thrust helpers (free functions)
    │   └── Collision.hpp         Circle-circle intersection (free functions)
    │
    ├── Wave.hpp / Wave.cpp       Wave count, asteroid spawn logic, inter-wave delay
    └── ScoreTable.hpp / .cpp     Top-5 session score table
```

**Namespace:** all project code lives in namespace `ast`. No sub-namespaces — the project
is small enough that the directory structure provides sufficient organisation.

---

## 3. Key Interfaces

The pure interfaces live in `game/` (no SDL2 dependency). Concrete implementations live in
their respective module directories and depend on SDL2 / SDL2_mixer.

### `IInputSource`

```cpp
struct InputState {
    bool thrust      = false;
    bool rotateLeft  = false;
    bool rotateRight = false;
    bool fire        = false;
    bool hyperspace  = false;
    bool start       = false;   // attract → playing
    bool connected   = false;
};

class IInputSource {
public:
    virtual ~IInputSource() = default;
    virtual InputState query() const = 0;
};
```

`Sdl2InputSource` reads from `SDL_GameController` or raw joystick. Button mapping is
**hardcoded** to the PiHut SNES layout (OQ-2 resolved: no config file in v1.0):

| Action | Button |
|--------|--------|
| Rotate left | D-pad left |
| Rotate right | D-pad right |
| Thrust | D-pad up or B (bottom face) |
| Fire | A (right face) |
| Hyperspace | X (top face) |
| Start | Start button |

### `IRenderer`

```cpp
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void clear(Colour background) = 0;
    virtual void drawLine(Vec2 a, Vec2 b, Colour c) = 0;
    virtual void drawLineStrip(std::span<const Vec2> points, Colour c, bool closed) = 0;
    virtual void present() = 0;
    virtual Vec2 screenSize() const = 0;
};
```

`closed = true` draws the final segment back to the first point (for polygons).
`MockRenderer` records all calls for assertion in unit tests.

HUD text (score, lives) is rendered with **line-drawn digits** (OQ-4 resolved: no SDL2_ttf
or bitmap font; 7-segment-style stroked numerals via `drawLine`). This keeps dependencies
minimal and matches the vector aesthetic.

### `IAudioSink`

```cpp
enum class SoundId {
    Thrust,           // looping
    Fire,
    ExplosionSmall,
    ExplosionLarge,   // ship or large/medium asteroid
    SaucerEngine,     // looping
    SaucerFire,
    BeatLow,
    BeatHigh,
};

class IAudioSink {
public:
    virtual ~IAudioSink() = default;
    virtual void play(SoundId id) = 0;
    virtual void loop(SoundId id) = 0;   // plays and repeats until stop()
    virtual void stop(SoundId id) = 0;
    virtual bool isPlaying(SoundId id) const = 0;
};
```

Audio is confirmed in v1.0 (OQ-6 resolved). `Sdl2AudioSink` wraps SDL2_mixer channels.
If SDL2_mixer initialisation fails, a `NullAudioSink` (no-op implementation) is substituted
silently — the game never knows audio is absent.

---

## 4. Entity Model

Entities are **value types stored in typed vectors** — no polymorphism, no heap allocation
per entity (OQ-1 entity storage resolved). Max entity counts at any moment are trivially
small (< 150), so vector iteration is cache-friendly and erase-remove is fast enough.

### `Vec2`

```cpp
struct Vec2 {
    float x = 0.f, y = 0.f;
    Vec2 operator+(Vec2 o) const;
    Vec2 operator*(float s) const;
    float length() const;
    Vec2 normalised() const;
    static float distance(Vec2 a, Vec2 b);
};
```

### `Ship`

```cpp
struct Ship {
    Vec2  position;
    Vec2  velocity;
    float angle          = 0.f;   // radians clockwise from screen-up
    float invincTimer    = 0.f;   // seconds; > 0 → immune; flashes when > 0
    bool  thrusting      = false; // drives thrust SFX and visual flame
    bool  active         = true;  // false during explosion/respawn sequence

    static constexpr float kRadius = 10.f;   // collision circle radius (px)
};
```

### `Asteroid`

```cpp
enum class AsteroidSize { Large, Medium, Small };

struct Asteroid {
    Vec2                  position;
    Vec2                  velocity;
    float                 angle        = 0.f;
    float                 angularVel   = 0.f;  // visual spin; no physics effect
    AsteroidSize          size;
    std::vector<Vec2>     shape;     // polygon vertices relative to centre; generated once at spawn

    float radius() const;           // Large=40, Medium=20, Small=10 (px)
};
```

Shape is generated randomly at spawn and stored — it does not change (OQ-1 resolved:
random shape per instance, fixed after construction). Tests seed the RNG to get
deterministic shapes.

### `Saucer`

```cpp
enum class SaucerSize { Large, Small };

struct Saucer {
    Vec2        position;
    Vec2        velocity;
    SaucerSize  size;
    float       fireTimer = 0.f;  // seconds until next shot
    bool        active    = false;

    float radius() const;   // Large=20, Small=10 (px)
};
```

At most one `Saucer` instance exists in `Game`; `active = false` when no saucer is
present. The global spawn timer lives in `Game`, not in `Saucer`.

### `Projectile`

```cpp
enum class ProjectileOwner { Player, Saucer };

struct Projectile {
    Vec2            position;
    Vec2            velocity;
    float           lifetime;   // seconds remaining; remove when ≤ 0
    ProjectileOwner owner;

    static constexpr float kRadius = 2.f;
};
```

Separate owner tag allows collision filtering without two separate vectors.

### `Particle`

```cpp
struct Particle {
    Vec2  position;
    Vec2  velocity;
    float lifetime;     // seconds remaining
    float maxLifetime;  // for alpha fade: alpha = lifetime / maxLifetime
};
```

---

## 5. Coordinate System

**Origin:** top-left corner, SDL2 native (OQ-3 resolved: no centre-origin conversion layer).

**Units:** screen pixels. No separate world-space scale.

**Y axis:** increases downward (SDL2 convention). Physics math uses this directly; no flip.
Rotation angles increase clockwise. Ship "up" (nose direction) is angle = 0, pointing
toward screen top (negative Y). Thrust vector: `Vec2(-sinf(angle), -cosf(angle)) * thrust`.

**Wrap-around:** applied after integration. If `x < 0`, `x += W`; if `x >= W`, `x -= W`;
same for Y. Applied to Ship, Asteroid, Saucer, Projectile. Particles do not wrap (they
are short-lived and stay near their origin).

**Screen size:** queried from `IRenderer::screenSize()` at startup; stored as `Vec2` in
`Game`. All spawn and wrap logic uses this value, not compile-time constants, so the game
scales to any resolution without code changes (NFR-3).

---

## 6. Game State Machine

Embedded as a private `enum class GameState` + transition logic inside `Game` (OQ-7
resolved: freestanding class is unnecessary overhead; transitions are tightly coupled to
entity management and cannot be sensibly decoupled).

```
            ┌───────────┐
  startup ─▶│  Attract  │◀────────────────────────────────────────┐
            └─────┬─────┘                                         │
                  │ START_PRESSED                                  │
                  │ (reset score/lives, spawn wave 1)             │
                  ▼                                               │
            ┌───────────┐   WAVE_CLEAR                            │
            │  Playing  │──────────────────┐                      │
            │           │◀──────────────── │ ──────────────────┐  │
            └─────┬─────┘  (inter-wave     │   RESPAWN_READY   │  │
                  │         delay, spawn   │   (timer elapsed, │  │
                  │         next wave)     │   spawn safe)     │  │
                  │ SHIP_DESTROYED                              │  │
                  │ (decrement life,                            │  │
                  │  begin explosion)                           │  │
                  ▼                                             │  │
            ┌────────────┐   lives > 0                         │  │
            │ PlayerDead │────────────────────────────────────▶┘  │
            └─────┬──────┘                                        │
                  │ lives == 0                                     │
                  ▼                                               │
            ┌──────────┐                                          │
            │ GameOver │──(delay elapsed or START_PRESSED)───────▶┘
            └──────────┘  (record score in ScoreTable)
```

**Wave clearing** does not leave the `Playing` state — it triggers an inter-wave delay
and re-spawn within the same state. The saucer spawn timer also runs entirely within
`Playing`; saucer appearance is not a state transition.

---

## 7. Data Flow — Main Game Loop

Update timestep is **fixed at 60 Hz, lock-step with rendering** (OQ-5 resolved:
RPi 400 can sustain 60 Hz for this workload; interpolation adds complexity for no
practical benefit). If a frame takes longer than 16.67 ms, the next frame catches up
with a larger `dt` (capped to avoid spiral-of-death).

```
main loop (target: 60 Hz)
│
├─ 1. Compute dt (capped at 50 ms to prevent spiral-of-death)
│
├─ 2. SDL_PollEvent loop
│       SDL_QUIT          → set quit flag
│       SDL_JOYDEVICEADDED/REMOVED → Sdl2InputSource::handleDeviceEvent()
│
├─ 3. InputState = input.query()
│
├─ 4. game.update(dt, inputState)
│       ├─ state machine transition checks
│       ├─ ship physics (thrust, drag, rotate, wrap)
│       ├─ asteroid update (move, wrap, spin)
│       ├─ saucer update (move, AI fire, wrap/exit)
│       ├─ projectile update (move, wrap, lifetime)
│       ├─ particle update (move, fade, expire)
│       ├─ collision detection (all active pairs)
│       ├─ spawn / destroy entities from collision results
│       └─ audio cue dispatch (IAudioSink calls)
│
├─ 5. renderer.clear(black)
│
├─ 6. game.render(renderer)
│       ├─ asteroids (polygon drawLineStrip)
│       ├─ saucer (polygon drawLineStrip)
│       ├─ ship (polygon drawLineStrip, flame if thrusting)
│       ├─ projectiles (drawLine short segment)
│       ├─ particles (drawLine short segment, alpha from lifetime)
│       └─ HUD (line-drawn score, ship-icon lives)
│
└─ 7. renderer.present()
       sleep until next tick
```

Audio cues are dispatched inside `update` (step 4), not during render. This keeps
`render` side-effect-free and deterministic.

---

## 8. Controller Hot-Plug Sequence

```
SDL_JOYDEVICEADDED event
  └─ Sdl2InputSource::handleDeviceEvent()
       ├─ SDL_JoystickOpen(device_index)
       └─ set connected_ = true

SDL_JOYDEVICEREMOVED event
  └─ Sdl2InputSource::handleDeviceEvent()
       ├─ SDL_JoystickClose(joystick_)
       └─ set connected_ = false

During disconnected frames:
  └─ query() returns InputState{} with connected = false
       └─ Game::update() sees connected == false
            └─ if Playing: ignores all action inputs (ship coasts)
               if Attract: waits silently for reconnect
```

No crash, no undefined behaviour. The saucer and asteroids continue moving during
disconnect (game world is not frozen) — the player simply cannot input.

---

## 9. Saucer AI

**Large saucer:** fires a projectile in a uniformly random direction at each fire interval.

**Small saucer:** fires toward the player ship with a configurable accuracy spread
(angle ± spread, where spread decreases in later waves). Parameters finalised at
feature selection.

Both saucers: enter from a randomly chosen left or right screen edge at a random Y
position, travel horizontally (with optional vertical weave for small saucer — TBD at
feature selection), exit the opposite edge. A new spawn timer starts when the previous
saucer exits or is destroyed.

---

## 10. Physics Helpers

Free functions in `game/physics/Physics.hpp` (no class, no state):

```cpp
Vec2  integratePosition(Vec2 pos, Vec2 vel, float dt);
Vec2  applyThrust(Vec2 vel, float angle, float accel, float dt);
Vec2  applyDrag(Vec2 vel, float drag, float dt);     // drag: fraction per second (0–1)
Vec2  wrapPosition(Vec2 pos, Vec2 screenSize);
float wrapAngle(float angle);                         // keeps angle in [0, 2π)
```

Free functions in `game/physics/Collision.hpp`:

```cpp
bool circlesOverlap(Vec2 aPos, float aRadius, Vec2 bPos, float bRadius);
```

All collision checks in `Game::update()` use `circlesOverlap`. No AABB, no polygon
intersection — circle collision is sufficient for this game and is fast and testable.

---

## 11. Build & Dependency Structure

```
executable: asteroids
    ├── lib: game          (NO SDL2 / SDL2_mixer dependency — pure C++17)
    ├── lib: rendering     (SDL2)
    ├── lib: input         (SDL2)
    ├── lib: audio         (SDL2_mixer)
    └── lib: platform      (SDL2)

test executable: asteroids_tests
    ├── lib: game          (same CMake target — no recompile)
    ├── MockRenderer       (GoogleMock impl of IRenderer)
    ├── MockInputSource    (GoogleMock impl of IInputSource)
    ├── MockAudioSink      (GoogleMock impl of IAudioSink)
    └── GoogleTest + GoogleMock (via FetchContent)
```

`lib: game` is the architectural firewall. It must never acquire an SDL2 or SDL2_mixer
`target_link_libraries` dependency. CI validates this implicitly: the GitHub Actions
Ubuntu runner runs unit tests without SDL2 installed.

---

## 12. Memory Ownership

| Object | Owner | Mechanism |
|--------|-------|-----------|
| `Platform` | `main()` stack | RAII destructor calls `SDL_Quit` |
| `SDL_Window*` | `Platform` | `unique_ptr` with custom deleter |
| `SDL_Renderer*` | `Sdl2Renderer` | `unique_ptr` with custom deleter |
| `Sdl2Renderer` | `main()` stack | RAII |
| `Sdl2InputSource` | `main()` stack | RAII |
| `Sdl2AudioSink` | `main()` stack | RAII |
| `Game` | `main()` stack | Receives `IRenderer&`, `IInputSource&`, `IAudioSink&` — non-owning |
| Entity vectors | `Game` | `std::vector<T>` value semantics |
| Asteroid shapes | `Asteroid` | `std::vector<Vec2>` member |

`new` and `delete` do not appear in any game code. Raw owning pointers do not exist.

---

## 13. Open Questions Resolution Summary

| ID | Question | Decision |
|----|----------|---------|
| OQ-1 | Asteroid shape | Random polygon generated at spawn, stored in `Asteroid::shape`; RNG seeded for test reproducibility |
| OQ-2 | Input mapping | Hardcoded PiHut SNES layout; no config file in v1.0 |
| OQ-3 | Coordinate origin | Top-left throughout; no conversion layer |
| OQ-4 | HUD font | Line-drawn 7-segment-style digits via `IRenderer::drawLine`; no SDL2_ttf |
| OQ-5 | Update/render coupling | Fixed 60 Hz lock-step; dt capped at 50 ms |
| OQ-6 | Audio | SDL2_mixer in v1.0; `NullAudioSink` fallback on init failure |
| OQ-7 | State machine | Embedded in `Game`; no freestanding class |

---

## 14. Document Status

| Section | Completeness |
|---------|-------------|
| System context | Final |
| Module breakdown & file layout | Final |
| Key interfaces (`IInputSource`, `IRenderer`, `IAudioSink`) | Final |
| Entity model (all fields) | Final |
| Coordinate system | Final |
| Game state machine | Final |
| Data flow / game loop | Final |
| Controller hot-plug sequence | Final |
| Saucer AI | Outline — parameters (fire rate, accuracy, weave) deferred to feature selection |
| Physics helpers | Final |
| Build & dependency structure | Final |
| Memory ownership | Final |
| Open questions | All resolved |
