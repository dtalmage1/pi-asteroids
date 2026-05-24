# Changelog

All notable changes to this project are documented here.
Format: one entry per merged PR, newest first.

---

## [Unreleased]

### Added
- INF-1: CMake project skeleton — SDL2 (release-2.30.9) and SDL2_mixer (release-2.8.0,
  WAV only) via FetchContent, both static; five library targets (`lib_game` with no SDL2
  dep, `lib_platform`, `lib_rendering`, `lib_input`, `lib_audio`); `asteroids`
  executable exits 0; `tests/` scaffold; `.gitignore`. SDL2_mixer target resolved via
  probe loop (`SDL2_mixer::SDL2_mixer-static`). Verified: MSVC/Windows host.
