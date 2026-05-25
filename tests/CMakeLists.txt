include(FetchContent)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
# Prevent GoogleTest from overriding compiler/linker settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    unit/placeholder_test.cpp
    unit/Vec2Test.cpp
    unit/InputStateTest.cpp
    unit/AudioSinkTest.cpp
    unit/GameTest.cpp
    unit/PhysicsTest.cpp
    unit/CollisionTest.cpp
    unit/ShipTest.cpp
    unit/AsteroidTest.cpp
    unit/ProjectileTest.cpp
    unit/ScoreTest.cpp
    unit/WaveTest.cpp
)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/unit)
target_link_libraries(unit_tests PRIVATE
    gtest_main
    gmock
    lib_game
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

if(CLANG_TIDY_EXE)
    set_target_properties(unit_tests PROPERTIES CXX_CLANG_TIDY "${_CT_CMD}")
endif()
