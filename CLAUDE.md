# Asteroids — Project Context for Claude Code

## Project Goal

A faithful Asteroids arcade clone running on a Raspberry Pi 400, controlled via the
PiHut wireless USB gamepad. Development is on a Windows laptop with SSH access to the Pi.

In the course of this development the following good coding practices will be followed:

- **Plan** — all steps and context needed for the development (this file is the first part)
- **Architecture** — designed and documented before any code is written
- **Requirements** — functional and non-functional, defined before implementation
- **Code analysis** — static analysis and review appropriate to the rigour of this project
- **Test cases** — unit tests (mocked hardware) and integration tests (real hardware on RPi)
- **CI pipeline** — runs on every commit; automates as many tests as possible; manual tests
  permitted where hardware-in-the-loop is unavoidable
- **Release procedure** — versioned, tagged, with changelog

---

## Claude Code Settings

`.claude/settings.json` contains the permission allowlist for development commands used in
this project: `cmake`, `ctest`, `git`, `rsync`, `ssh` (including `ssh dan@dtdan`), and the
full Windows path to cmake (`C:\Program Files\CMake\bin\cmake.exe`). These are pre-approved
so Claude Code does not prompt for each invocation.

---

## Development Environment

### Host (Development Machine)
- **OS:** Windows (PowerShell)
- **IDE:** VSCode with Claude Code extension
- **Languages:** C++17 (primary), Python, C

### Target (Runtime Hardware)
- **Device:** Raspberry Pi 400
- **OS:** Raspberry Pi OS (Bookworm, 64-bit)
- **Hostname:** `dtdan`
- **User:** `dan`
- **SSH:** `ssh dan@dtdan` (key auth, no password prompt)

### Controller
- **Device:** PiHut Wireless USB Game Controller (SNES-style)
- **Interface:** `/dev/input/js0` (Linux joystick API)
- **Test command:** `jstest /dev/input/js0`

### Audio
- **Device:** `plughw:1,0` (HDMI card 1)
- **Confirmed:** sine wave test passed at 48000Hz stereo
---

## Toolchain Decisions

| Concern         | Decision              | Rationale                                              |
|-----------------|-----------------------|--------------------------------------------------------|
| Language        | C++17                 | Developer's strongest language; suits project scale    |
| Rendering/Input | SDL2                  | Well supported on RPi; handles display, input, timing  |
| Build system    | CMake                 | Industry standard; FetchContent for dependencies       |
| Test framework  | GoogleTest + GoogleMock | Clean hardware interface mocking; mature CI support  |
| Repo & CI host  | GitHub + GitHub Actions | Existing account; good ARM/RPi CI examples           |
| Static analysis | clang-tidy            | Integrates with CMake; enforces C++17 best practices   |

---

## Workflow

### Deploy to RPi
```bash
rsync -avz --exclude='.git' ./ dan@dtdan:~/Documents/Projects/Asteroids/
```

### Build on RPi
```bash
ssh dan@dtdan "cd ~/Documents/Projects/Asteroids && cmake -B build && cmake --build build"
```

### Run on RPi
```bash
ssh dan@dtdan "cd ~/Documents/Projects/Asteroids/build && ./asteroids"
```

### Run Tests (host, mocked hardware)
```bash
cmake -B build && cmake --build build --target tests && ctest --output-on-failure
```

### Run Tests (RPi, real hardware)
```bash
ssh dan@dtdan "cd ~/Documents/Projects/Asteroids/build && ctest --output-on-failure"
```

---

## Project Structure

```
Asteroids/
├── CLAUDE.md                   # This file — Claude Code project context
├── README.md
├── CMakeLists.txt              # Top-level build definition
├── docs/
│   ├── plan.md                 # Development plan and milestones
│   ├── requirements.md         # Functional & non-functional requirements
│   ├── architecture.md         # System architecture & design decisions
│   └── release.md              # Release procedure and checklist
├── src/
│   ├── main.cpp
│   ├── game/                   # Game logic (physics, entities, state machine)
│   ├── rendering/              # SDL2 rendering abstraction
│   ├── input/                  # Joystick / input abstraction
│   └── platform/               # Platform-specific helpers
├── tests/
│   ├── unit/                   # GoogleTest unit tests (mocked hardware)
│   └── integration/            # Hardware-in-the-loop tests (run on RPi)
├── scripts/
│   ├── deploy.sh               # rsync deploy to RPi
│   └── run_integration_tests.sh
└── .github/
    └── workflows/
        └── ci.yml              # GitHub Actions CI pipeline
```

---

## Coding Standards

### C++17
- RAII throughout; no raw owning pointers (`std::unique_ptr` / `std::shared_ptr`)
- Prefer `std::` over C APIs wherever available
- No magic numbers — named constants or `constexpr`
- No dead code committed
- All hardware interfaces (joystick, renderer) behind abstract interfaces to enable mocking
- `clang-tidy` must pass with no warnings before merge

### Git Conventions
- Branch naming: `feature/<name>`, `fix/<name>`, `chore/<name>`
- Commit messages: imperative mood, present tense (`Add player ship rendering`)
- No direct commits to `main`; feature branches + PR merge only
- Every PR must pass CI before merge

---

## CI Pipeline (GitHub Actions)

Runs on every commit and PR to `main`:

1. **Build** — CMake configure + build on Ubuntu runner (host build)
2. **Lint** — `clang-tidy` static analysis
3. **Unit tests** — GoogleTest suite with mocked hardware (runs on GitHub runner)
4. **Deploy & integration tests** — rsync to RPi, run hardware-in-the-loop tests
   *(manual trigger or self-hosted runner on RPi; automated where possible)*
5. **Release** — on version tag: build release artifact, update changelog, create GitHub Release

---

## Release Procedure

1. All CI stages passing on `main`
2. Version bumped (`constexpr` in source + `CMakeLists.txt`)
3. `CHANGELOG.md` updated
4. Git tag created: `v<major>.<minor>.<patch>`
5. GitHub Actions builds and publishes release artifact automatically

---

## Key Constraints & Notes

- RPi 400 has VideoCore VI but no desktop GPU; SDL2 software renderer is sufficient for
  Asteroids-scale vector graphics
- Controller is wireless USB — code must handle connect/disconnect gracefully
- All AI processing is remote (Anthropic cloud) — the RPi runs only compiled project code
- Dan is an experienced embedded/low-level C++ developer — no need to explain language basics
