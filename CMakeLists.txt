cmake_minimum_required(VERSION 3.25)

# Project metadata is derived by the included project definition from VERSION.txt
# through PHYSICS_SIM_VERSION_FULL and PHYSICS_SIM_VERSION_CORE.
include("${CMAKE_CURRENT_LIST_DIR}/cmake/physics_sim_project.cmake")

# The version/packaging branch started before runtime checkpoints landed. Keep the
# checkpoint target additive here so the version refactor cannot remove current
# mainline coverage while the shared project definition is introduced.
if(BUILD_TESTING)
    add_executable(physics_sim_runtime_checkpoint_tests
        tests/runtime_checkpoint_tests.cpp
    )
    target_link_libraries(physics_sim_runtime_checkpoint_tests PRIVATE physics_sim_core)
    target_compile_features(physics_sim_runtime_checkpoint_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(physics_sim_runtime_checkpoint_tests PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(physics_sim_runtime_checkpoint_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()
    add_test(NAME physics_sim_runtime_checkpoint_tests COMMAND physics_sim_runtime_checkpoint_tests)
    set_tests_properties(physics_sim_runtime_checkpoint_tests PROPERTIES LABELS "integration")
endif()
