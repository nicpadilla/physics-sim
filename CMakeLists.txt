cmake_minimum_required(VERSION 3.25)

project(physics_sim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_library(physics_sim_core STATIC
    src/core/simulation.cpp
)
target_include_directories(physics_sim_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(physics_sim_core PUBLIC cxx_std_20)

add_library(physics_sim_content STATIC
    src/content/content_module.cpp
)
target_link_libraries(physics_sim_content PUBLIC physics_sim_core)
target_compile_features(physics_sim_content PUBLIC cxx_std_20)

add_library(physics_sim_app STATIC
    src/app/application.cpp
    src/app/surface_reconstruction.cpp
)
target_link_libraries(physics_sim_app PUBLIC physics_sim_content physics_sim_core SDL2::SDL2)
target_compile_features(physics_sim_app PUBLIC cxx_std_20)

add_library(physics_sim_lab STATIC
    src/lab/lab_module.cpp
)
target_link_libraries(physics_sim_lab PUBLIC physics_sim_content physics_sim_core SDL2::SDL2 imgui::imgui)
target_compile_features(physics_sim_lab PUBLIC cxx_std_20)

add_executable(physics_sim WIN32
    src/main.cpp
)

target_link_libraries(physics_sim
    PRIVATE
        physics_sim_app
        physics_sim_lab
)

target_compile_features(physics_sim PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(physics_sim PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_options(physics_sim_core PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_options(physics_sim_content PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_options(physics_sim_app PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_options(physics_sim_lab PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_definitions(physics_sim PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_compile_definitions(physics_sim_core PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_compile_definitions(physics_sim_app PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_compile_definitions(physics_sim_lab PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

set_target_properties(physics_sim PROPERTIES
    OUTPUT_NAME "physics-sim"
)

if(BUILD_TESTING)
    add_executable(physics_sim_simulation_api_tests
        tests/simulation_api_tests.cpp
    )
    target_link_libraries(physics_sim_simulation_api_tests PRIVATE physics_sim_core)
    target_compile_features(physics_sim_simulation_api_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(physics_sim_simulation_api_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_simulation_api_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()
    add_test(NAME physics_sim_simulation_api_tests COMMAND physics_sim_simulation_api_tests)
    add_executable(physics_sim_surface_reconstruction_tests
        tests/surface_reconstruction_tests.cpp
    )
    target_link_libraries(physics_sim_surface_reconstruction_tests PRIVATE physics_sim_app)
    target_compile_features(physics_sim_surface_reconstruction_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(physics_sim_surface_reconstruction_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_surface_reconstruction_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()
    add_test(NAME physics_sim_surface_reconstruction_tests COMMAND physics_sim_surface_reconstruction_tests)
    add_test(NAME physics_sim_dependency_check
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check-dependencies.ps1
    )

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

    add_executable(physics_sim_ui_tests
        tests/visual_mode_tests.cpp
        tests/debug_overlay_tests.cpp
    )

    target_link_libraries(physics_sim_ui_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_ui_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_ui_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_ui_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_ui_tests COMMAND physics_sim_ui_tests)

    add_executable(physics_sim_feedback_tests
        tests/feedback_tests.cpp
    )

    target_link_libraries(physics_sim_feedback_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_feedback_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_feedback_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_feedback_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_feedback_tests COMMAND physics_sim_feedback_tests)

    add_executable(physics_sim_ui_palette_tests
        tests/ui_palette_tests.cpp
    )

    target_link_libraries(physics_sim_ui_palette_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_ui_palette_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_ui_palette_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_ui_palette_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_ui_palette_tests COMMAND physics_sim_ui_palette_tests)

    add_executable(physics_sim_player_feedback_tests
        tests/player_feedback_tests.cpp
    )

    target_link_libraries(physics_sim_player_feedback_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_player_feedback_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_player_feedback_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_player_feedback_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_player_feedback_tests COMMAND physics_sim_player_feedback_tests)

    add_executable(physics_sim_save_browser_tests
        tests/save_browser_tests.cpp
    )

    target_link_libraries(physics_sim_save_browser_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_save_browser_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_save_browser_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_save_browser_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_save_browser_tests COMMAND physics_sim_save_browser_tests)

    add_executable(physics_sim_audio_feedback_tests
        tests/audio_feedback_tests.cpp
    )

    target_link_libraries(physics_sim_audio_feedback_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_audio_feedback_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_audio_feedback_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_audio_feedback_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_audio_feedback_tests COMMAND physics_sim_audio_feedback_tests)

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

    add_executable(physics_sim_scene_gallery_tests
        tests/scene_gallery_tests.cpp
    )

    target_link_libraries(physics_sim_scene_gallery_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_scene_gallery_tests PRIVATE cxx_std_20)

    target_compile_definitions(physics_sim_scene_gallery_tests PRIVATE
        PHYSICS_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    if(MSVC)
        target_compile_options(physics_sim_scene_gallery_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_scene_gallery_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_scene_gallery_tests COMMAND physics_sim_scene_gallery_tests)

    add_executable(physics_sim_device_runtime_tests
        tests/device_runtime_tests.cpp
    )

    target_link_libraries(physics_sim_device_runtime_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_device_runtime_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_device_runtime_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_device_runtime_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_device_runtime_tests COMMAND physics_sim_device_runtime_tests)

    add_executable(physics_sim_replay_script_tests
        tests/replay_script_tests.cpp
    )

    target_link_libraries(physics_sim_replay_script_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_replay_script_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_replay_script_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_replay_script_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_replay_script_tests COMMAND physics_sim_replay_script_tests)

    add_executable(physics_sim_user_settings_tests
        tests/user_settings_tests.cpp
    )

    target_link_libraries(physics_sim_user_settings_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_user_settings_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_user_settings_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_user_settings_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_user_settings_tests COMMAND physics_sim_user_settings_tests)

    add_executable(physics_sim_input_bindings_tests
        tests/input_bindings_tests.cpp
    )

    target_link_libraries(physics_sim_input_bindings_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_input_bindings_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_input_bindings_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_input_bindings_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_input_bindings_tests COMMAND physics_sim_input_bindings_tests)

    add_executable(physics_sim_player_guidance_tests
        tests/player_guidance_tests.cpp
    )

    target_link_libraries(physics_sim_player_guidance_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_player_guidance_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_player_guidance_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_player_guidance_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_player_guidance_tests COMMAND physics_sim_player_guidance_tests)

    add_executable(physics_sim_settings_menu_tests
        tests/settings_menu_tests.cpp
    )

    target_link_libraries(physics_sim_settings_menu_tests
        PRIVATE
            physics_sim_core
            SDL2::SDL2
    )

    target_compile_features(physics_sim_settings_menu_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_settings_menu_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_settings_menu_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_settings_menu_tests COMMAND physics_sim_settings_menu_tests)

    add_executable(physics_sim_session_shell_tests
        tests/session_shell_tests.cpp
    )

    target_link_libraries(physics_sim_session_shell_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_session_shell_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_session_shell_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_session_shell_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_session_shell_tests COMMAND physics_sim_session_shell_tests)

    add_executable(physics_sim_tutorial_tests
        tests/tutorial_progress_tests.cpp
    )

    target_link_libraries(physics_sim_tutorial_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_tutorial_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_tutorial_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_tutorial_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_tutorial_tests COMMAND physics_sim_tutorial_tests)

    add_executable(physics_sim_tutorial_scene_tests
        tests/tutorial_scene_tests.cpp
    )

    target_link_libraries(physics_sim_tutorial_scene_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_tutorial_scene_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_tutorial_scene_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_tutorial_scene_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    target_compile_definitions(physics_sim_tutorial_scene_tests PRIVATE PHYSICS_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME physics_sim_tutorial_scene_tests COMMAND physics_sim_tutorial_scene_tests)

    add_executable(physics_sim_fluid_quality_tests
        tests/fluid_quality_tests.cpp
    )

    target_link_libraries(physics_sim_fluid_quality_tests
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_fluid_quality_tests PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_fluid_quality_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_fluid_quality_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_fluid_quality_tests COMMAND physics_sim_fluid_quality_tests)

    add_executable(physics_sim_solver_benchmark
        tests/water_solver_benchmark.cpp
    )

    target_link_libraries(physics_sim_solver_benchmark
        PRIVATE
            physics_sim_core
    )

    target_compile_features(physics_sim_solver_benchmark PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(physics_sim_solver_benchmark PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_solver_benchmark PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    add_test(NAME physics_sim_solver_benchmark COMMAND physics_sim_solver_benchmark)

    add_test(NAME physics_sim_demo_regression
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify-demo-scene.ps1
    )

    add_test(NAME physics_sim_demo_regression_density
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify-demo-scene-density.ps1
    )

    add_test(NAME physics_sim_demo_regression_surface
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify-demo-scene-surface.ps1
    )
    add_test(NAME physics_sim_smoke
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-smoke.ps1 -SkipBuild
    )
    add_test(NAME physics_sim_lab_smoke
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-lab-smoke.ps1 -SkipBuild
    )
    add_test(NAME physics_sim_package
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/package-release.ps1 -SkipBuild
    )

    set_tests_properties(
        physics_sim_simulation_api_tests
        physics_sim_surface_reconstruction_tests
        physics_sim_dependency_check
        physics_sim_tests
        physics_sim_wall_tests
        physics_sim_ui_tests
        physics_sim_feedback_tests
        physics_sim_ui_palette_tests
        physics_sim_player_feedback_tests
        physics_sim_save_browser_tests
        physics_sim_audio_feedback_tests
        physics_sim_replay_script_tests
        physics_sim_user_settings_tests
        physics_sim_input_bindings_tests
        physics_sim_player_guidance_tests
        physics_sim_settings_menu_tests
        physics_sim_session_shell_tests
        physics_sim_tutorial_tests
        PROPERTIES LABELS "unit"
    )
    set_tests_properties(
        physics_sim_editor_tests
        physics_sim_scene_persistence_tests
        physics_sim_scene_gallery_tests
        physics_sim_device_runtime_tests
        physics_sim_tutorial_scene_tests
        PROPERTIES LABELS "integration"
    )
    set_tests_properties(
        physics_sim_water_tests
        physics_sim_fluid_quality_tests
        PROPERTIES LABELS "solver"
    )
    set_tests_properties(physics_sim_solver_benchmark PROPERTIES LABELS "benchmark")
    set_tests_properties(physics_sim_smoke physics_sim_lab_smoke PROPERTIES LABELS "smoke")
    set_tests_properties(physics_sim_package PROPERTIES LABELS "release")
    set_tests_properties(
        physics_sim_demo_regression
        physics_sim_demo_regression_density
        physics_sim_demo_regression_surface
        PROPERTIES LABELS "visual"
    )
endif()
