# Changelog

All notable changes to this project are documented here.
Format: one entry per merged PR, newest first.

---

## [Unreleased]

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
