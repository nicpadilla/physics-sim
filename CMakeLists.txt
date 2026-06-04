cmake_minimum_required(VERSION 3.25)

project(physics_sim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

find_package(SDL2 CONFIG REQUIRED)

add_library(physics_sim_core INTERFACE)
target_include_directories(physics_sim_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(physics_sim WIN32
    src/main.cpp
)

target_link_libraries(physics_sim
    PRIVATE
        physics_sim_core
        SDL2::SDL2
)

target_compile_features(physics_sim PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(physics_sim PRIVATE /W4 /permissive- /utf-8)
    target_compile_options(physics_sim_core INTERFACE /W4 /permissive- /utf-8)
    target_compile_definitions(physics_sim PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

set_target_properties(physics_sim PROPERTIES
    OUTPUT_NAME "physics-sim"
)

if(BUILD_TESTING)
    add_executable(physics_sim_tests
        tests/math_tests.cpp
    )

    target_link_libraries(physics_sim_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_tests COMMAND physics_sim_tests)

    add_executable(physics_sim_water_tests
        tests/water_simulation_tests.cpp
    )

    target_link_libraries(physics_sim_water_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_water_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_water_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_water_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_water_tests COMMAND physics_sim_water_tests)

    add_executable(physics_sim_wall_tests
        tests/wall_editing_tests.cpp
    )

    target_link_libraries(physics_sim_wall_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_wall_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_wall_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_wall_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_wall_tests COMMAND physics_sim_wall_tests)

    add_executable(physics_sim_editor_tests
        tests/editor_controls_tests.cpp
    )

    target_link_libraries(physics_sim_editor_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_editor_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_editor_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_editor_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_editor_tests COMMAND physics_sim_editor_tests)

    add_executable(physics_sim_scene_persistence_tests
        tests/scene_persistence_tests.cpp
    )

    target_link_libraries(physics_sim_scene_persistence_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_scene_persistence_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_scene_persistence_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_scene_persistence_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_scene_persistence_tests COMMAND physics_sim_scene_persistence_tests)

    add_test(NAME physics_sim_demo_regression
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify-demo-scene.ps1
    )
endif()
