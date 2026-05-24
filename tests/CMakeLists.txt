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
)
target_link_libraries(unit_tests PRIVATE
    gtest_main
    gmock
    lib_game
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
