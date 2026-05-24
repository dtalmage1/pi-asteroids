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

## 5. Out of Scope for v1.0

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
