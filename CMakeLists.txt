cmake_minimum_required(VERSION 3.24)
project(asteroids VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build all FetchContent dependencies as static libraries.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Export compile_commands.json for clangd / IDE integration (Ninja/Makefile only).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ── SDL2 ──────────────────────────────────────────────────────────────────────
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL2)

# ── SDL2_mixer (WAV support only) ─────────────────────────────────────────────
set(SDL2MIXER_INSTALL  OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES  OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD      OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3      OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OGG      OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI     OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVPACK  OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-2.8.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL2_mixer)

# ── Game library — NO SDL2 dependency ─────────────────────────────────────────
# All game logic lives here so it can be unit-tested without SDL2 present.
add_library(lib_game STATIC
    src/game/Game.cpp
    src/game/Glyph.cpp
    src/game/ScoreTable.cpp
    src/game/Wave.cpp
    src/game/entities/Asteroid.cpp
    src/game/entities/Saucer.cpp
)
target_include_directories(lib_game PUBLIC src)

# ── Platform library ──────────────────────────────────────────────────────────
add_library(lib_platform STATIC
    src/platform/Platform.cpp
)
target_include_directories(lib_platform PUBLIC src)
target_link_libraries(lib_platform PRIVATE SDL2::SDL2-static)

# ── Rendering library ─────────────────────────────────────────────────────────
add_library(lib_rendering STATIC
    src/rendering/Sdl2Renderer.cpp
)
target_include_directories(lib_rendering PUBLIC src)
target_link_libraries(lib_rendering PRIVATE SDL2::SDL2-static)

# ── Input library ─────────────────────────────────────────────────────────────
add_library(lib_input STATIC
    src/input/Sdl2InputSource.cpp
)
target_include_directories(lib_input PUBLIC src)
target_link_libraries(lib_input PRIVATE SDL2::SDL2-static)

# ── Audio library ─────────────────────────────────────────────────────────────
add_library(lib_audio STATIC
    src/audio/Sdl2AudioSink.cpp
)
target_include_directories(lib_audio PUBLIC src)
# Target name differs between static and shared builds of SDL2_mixer.
# Probe for the correct target rather than hard-coding one name.
foreach(_t SDL2_mixer::SDL2_mixer-static SDL2_mixer::SDL2_mixer SDL2_mixer)
    if(TARGET ${_t})
        set(SDL2MIXER_TARGET ${_t})
        break()
    endif()
endforeach()
if(NOT SDL2MIXER_TARGET)
    message(FATAL_ERROR "Could not find a linkable SDL2_mixer target")
endif()
target_link_libraries(lib_audio PRIVATE ${SDL2MIXER_TARGET} SDL2::SDL2-static)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(asteroids src/main.cpp)
target_link_libraries(asteroids PRIVATE
    lib_game
    lib_platform
    lib_rendering
    lib_input
    lib_audio
    SDL2::SDL2-static
)

# ── clang-tidy (applied to project targets only, not FetchContent deps) ───────
option(ENABLE_CLANG_TIDY "Run clang-tidy during build" ON)
file(GLOB _CT_HINTS
    "C:/Program Files/Microsoft Visual Studio/*/*/VC/Tools/Llvm/x64/bin"
    "C:/Program Files/LLVM/bin"
)
find_program(CLANG_TIDY_EXE NAMES clang-tidy HINTS ${_CT_HINTS})
if(CLANG_TIDY_EXE AND ENABLE_CLANG_TIDY)
    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
    set(_CT_CMD "${CLANG_TIDY_EXE}")
    set_target_properties(
        lib_game lib_platform lib_rendering lib_input lib_audio asteroids
        PROPERTIES CXX_CLANG_TIDY "${_CT_CMD}"
    )
elseif(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy: found but disabled (ENABLE_CLANG_TIDY=OFF)")
else()
    message(WARNING "clang-tidy not found — static analysis disabled")
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
