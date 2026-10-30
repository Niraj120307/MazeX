# ============================================================================
#  MAZE-X  -  CMake build
#
#    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#    cmake --build build -j
#
#  Targets:  MazeX (SFML GUI)   mazex_console   mazex_tests   (ctest runs the tests)
#
#  SFML: uses an installed SFML 3 if CMake can find it (e.g. MSYS2), otherwise
#  downloads SFML 3.0.2 and builds it with YOUR compiler, so there is no
#  "compiler version must match 100%" problem.
#  Console + tests only (no SFML at all):  -DMAZEX_BUILD_GUI=OFF
# ============================================================================
cmake_minimum_required(VERSION 3.24)
project(MazeX VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

option(MAZEX_BUILD_GUI "Build the SFML graphical interface" ON)

if(MSVC)
    set(MAZEX_WARNINGS /W4 /permissive-)
else()
    set(MAZEX_WARNINGS -Wall -Wextra -pedantic)
endif()

# ---- Core engine (no graphics) ---------------------------------------------
add_library(mazex_core STATIC
    src/Maze.cpp src/MazeIO.cpp src/SolverResult.cpp
    src/DFS.cpp src/BFS.cpp src/Dijkstra.cpp src/AStar.cpp
    src/MazeGenerator.cpp src/Playback.cpp src/Benchmark.cpp src/ConsoleView.cpp)
target_include_directories(mazex_core PUBLIC include)
target_compile_options(mazex_core PRIVATE ${MAZEX_WARNINGS})

add_executable(mazex_console src/main.cpp)
target_link_libraries(mazex_console PRIVATE mazex_core)
target_compile_options(mazex_console PRIVATE ${MAZEX_WARNINGS})

add_executable(mazex_tests tests/test_main.cpp tests/test_maze.cpp tests/test_solvers.cpp tests/test_engine.cpp)
target_include_directories(mazex_tests PRIVATE tests)
target_link_libraries(mazex_tests PRIVATE mazex_core)
target_compile_options(mazex_tests PRIVATE ${MAZEX_WARNINGS})

enable_testing()
add_test(NAME mazex_tests COMMAND mazex_tests -q WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# ---- SFML GUI ---------------------------------------------------------------
if(MAZEX_BUILD_GUI)
    find_package(SFML 3 COMPONENTS Graphics QUIET)
    if(NOT SFML_FOUND)
        message(STATUS "SFML 3 not found - downloading and building SFML 3.0.2 (first build takes a few minutes)")
        include(FetchContent)
        set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 3.0.2
            GIT_SHALLOW ON
            EXCLUDE_FROM_ALL
            SYSTEM)
        FetchContent_MakeAvailable(SFML)
    endif()

    add_executable(MazeX
        src/ui/main_gui.cpp src/ui/App.cpp src/ui/Controls.cpp src/ui/Renderer.cpp src/ui/Widgets.cpp)
    target_link_libraries(MazeX PRIVATE mazex_core SFML::Graphics)
    target_compile_options(MazeX PRIVATE ${MAZEX_WARNINGS})

    # Put assets/ (fonts + demo maze) next to the exe so it runs from anywhere.
    add_custom_command(TARGET MazeX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:MazeX>/assets)
endif()
