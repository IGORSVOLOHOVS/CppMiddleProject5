cmake_minimum_required(VERSION 3.28)
project(GeometryApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# clang-tidy reads this: without it the FetchContent headers are not found.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The console demo is the only part that needs matplot++ and <print>. recipe.ps1
# switches it off, so the geometry core builds and tests with g++ 13 and no gnuplot.
option(GEOMETRY_APP_DEMO "Build GeometryApp, the demo that draws with matplot++" ON)
option(BUILD_TESTING "Build tests" ON)

function(warn_as_errors target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /bigobj)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_library(geometry_core STATIC src/convex_hull.cpp)
target_include_directories(geometry_core PUBLIC include)
warn_as_errors(geometry_core)

if(GEOMETRY_APP_DEMO)
    include(FetchContent)
    FetchContent_Declare(
        CPM
        GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake
        GIT_TAG origin/master
    )
    FetchContent_MakeAvailable(CPM)
    CPMAddPackage(
        NAME matplotplusplus
        GITHUB_REPOSITORY alandefreitas/matplotplusplus
        GIT_TAG master
    )

    add_executable(GeometryApp src/main.cpp src/visualization.cpp)
    target_include_directories(GeometryApp PRIVATE ${matplotplusplus_SOURCE_DIR}/source)
    target_link_libraries(GeometryApp PRIVATE geometry_core matplot)
endif()

if(BUILD_TESTING)
    enable_testing()

    # An archive and its hash, not a git clone: nothing here can ask for a credential on the runner.
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz
        URL_HASH SHA256=ad7fdba11ea011c1d925b3289cf4af2c66a352e18d4c7264392fead75e919363
        SYSTEM
    )
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(GeometryApp_tests
        tests/convex_hull_test.cpp
        tests/geometry_test.cpp
        tests/intersections_test.cpp
        tests/main.cpp
        tests/queries_test.cpp
        tests/shape_utils_test.cpp
        tests/triangulation_test.cpp
    )
    target_link_libraries(GeometryApp_tests PRIVATE geometry_core GTest::gtest)
    warn_as_errors(GeometryApp_tests)

    add_test(NAME geometry_tests COMMAND GeometryApp_tests)
    # The manual list is judged where every other file is judged (section 9).
    add_test(
        NAME manual_list
        COMMAND python -m pytest tests/test_manual_list.py -q -p no:cacheprovider
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
