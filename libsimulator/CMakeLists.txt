################################################################################
# Build libsimulator
################################################################################


add_library(simulator STATIC)

target_sources(simulator PRIVATE
    src/aabb.cpp
    src/aabb.hpp
    src/agent_removal_system.hpp
    src/agent_view.hpp
    src/cfg_cgal.hpp
    src/ellipse.cpp
    src/ellipse.hpp
    src/floorfield_routing_engine.cpp
    src/floorfield_routing_engine.hpp
    src/generic_agent.hpp
    src/geometric_functions.hpp
    src/destination.cpp
    src/destination.hpp
    src/geometry/area_piece.hpp
    src/geometry/boundary_index.cpp
    src/geometry/boundary_index.hpp
    src/geometry/geometry.cpp
    src/geometry/geometry.hpp
    src/geometry/location.cpp
    src/geometry/location.hpp
    src/geometry/projected_boundary.cpp
    src/geometry/projected_boundary.hpp
    src/geometry/region_split.cpp
    src/geometry/region_split.hpp
    src/geometry/segment_grid.cpp
    src/geometry/segment_grid.hpp
    src/geometry/validation.cpp
    src/geometry/validation.hpp
    src/geometry/walkable_surface.cpp
    src/geometry/walkable_surface.hpp
    src/geometry_builder.cpp
    src/geometry_builder.hpp
    src/hash_combine.hpp
    src/iterator_pair.hpp
    src/journey.cpp
    src/journey.hpp
    src/line_segment.cpp
    src/line_segment.hpp
    src/logger.cpp
    src/logger.hpp
    src/macros.hpp
    src/mathematics.cpp
    src/mathematics.hpp
    src/neighborhood_search.hpp
    src/operational_decision_system.hpp
    src/point.cpp
    src/point.hpp
    src/polygon.cpp
    src/polygon.hpp
    src/routing_engine.cpp
    src/routing_engine.hpp
    src/simulation.cpp
    src/simulation.hpp
    src/simulation_clock.cpp
    src/simulation_clock.hpp
    src/simulation_error.hpp
    src/stage.cpp
    src/stage.hpp
    src/stage_description.hpp
    src/stage_manager.cpp
    src/stage_manager.hpp
    src/strategical_desicion_system.hpp
    src/surface_mesh_shortest_path_routing_engine.cpp
    src/surface_mesh_shortest_path_routing_engine.hpp
    src/tactical_decision_system.hpp
    src/timing.cpp
    src/timing.hpp
    src/tracing.cpp
    src/tracing.hpp
    src/unique_id.hpp
    src/util.hpp
)

add_subdirectory(src/operational_models)
target_link_libraries(simulator PRIVATE project_options)
target_compile_definitions(simulator PUBLIC
    JPSCORE_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(simulator PUBLIC
    common
    fmt::fmt
    CGAL::CGAL
    build_info
    glm::glm
    perfetto
    floorfield_cxx
)
target_link_options(simulator PUBLIC
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>,$<BOOL:${BUILD_WITH_SANITIZERS}>>:-fsanitize=address,undefined>
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<BOOL:${BUILD_WITH_SANITIZERS}>>:-shared-libasan>
    $<$<CXX_COMPILER_ID:GNU>:-Werror=odr>  # error out if LTO finds multiple definitions
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

################################################################################
# libsimulator unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(libsimulator-tests
        test/geometry_fixtures.hpp
        test/test_aabb.cpp
        test/test_agent_view.cpp
        test/test_basic_primitive_tests.cpp
        test/test_boundary_index.cpp
        test/test_common.hpp
        test/test_custom_model.cpp
        test/test_floorfield.cpp
        test/test_generic_agent_formatter.cpp
        test/test_geometry.cpp
        test/test_journey.cpp
        test/test_line_segment.cpp
        test/test_location.cpp
        test/test_neighborhood_search.cpp
        test/test_operational_decision_system.cpp
        test/test_point.cpp
        test/test_projected_boundary.cpp
        test/test_region_split.cpp
        test/test_routing_engine.cpp
        test/test_segment_grid.cpp
        test/test_simulation.cpp
        test/test_simulation_clock.cpp
        test/test_stage.cpp
        test/test_surface_mesh_shortest_path_routing_engine.cpp
        test/test_unique_id.cpp
        test/test_validation.cpp
        test/test_walkable_surface.cpp
    )

    target_link_libraries(libsimulator-tests PRIVATE
        GTest::gmock
        GTest::gtest_main
        GTest::gtest
        simulator
    )

    target_link_libraries(libsimulator-tests PRIVATE project_options)

endif()

################################################################################
# libjupedsim micro benchmarks
################################################################################
if (BUILD_BENCHMARKS)
    add_executable(libsimulator-benchmarks
        benchmark/benchmark_main.cpp
        benchmark/benchmark_line_segment.hpp
        benchmark/build_geometries.hpp
    )

    target_link_libraries(libsimulator-benchmarks PRIVATE
        benchmark::benchmark
        simulator
    )

    target_link_libraries(libsimulator-benchmarks PRIVATE project_options)

endif ()
