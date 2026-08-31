################################################################################
# Build libsimulator
################################################################################


add_library(simulator STATIC)

target_sources(simulator PRIVATE
    src/AABB.cpp
    src/AABB.hpp
    src/AgentRemovalSystem.hpp
    src/AgentView.hpp
    src/CfgCgal.hpp
    src/Ellipse.cpp
    src/Ellipse.hpp
    src/GenericAgent.hpp
    src/GeometricFunctions.hpp
    src/Geometry/BoundaryIndex.cpp
    src/Geometry/BoundaryIndex.hpp
    src/Geometry/Geometry.cpp
    src/Geometry/Geometry.hpp
    src/Geometry/Location.cpp
    src/Geometry/Location.hpp
    src/Geometry/ProjectedBoundary.cpp
    src/Geometry/ProjectedBoundary.hpp
    src/Geometry/RegionSeams.cpp
    src/Geometry/RegionSeams.hpp
    src/Geometry/RegionSplit.cpp
    src/Geometry/RegionSplit.hpp
    src/Geometry/RegionView.cpp
    src/Geometry/RegionView.hpp
    src/Geometry/SegmentGrid.cpp
    src/Geometry/SegmentGrid.hpp
    src/Geometry/Validation.cpp
    src/Geometry/Validation.hpp
    src/GeometryBuilder.cpp
    src/GeometryBuilder.hpp
    src/HashCombine.hpp
    src/IteratorPair.hpp
    src/Journey.cpp
    src/Journey.hpp
    src/LineSegment.cpp
    src/LineSegment.hpp
    src/Logger.cpp
    src/Logger.hpp
    src/Macros.hpp
    src/Mathematics.cpp
    src/Mathematics.hpp
    src/NeighborhoodSearch.hpp
    src/OperationalDecisionSystem.hpp
    src/Point.cpp
    src/Point.hpp
    src/Polygon.cpp
    src/Polygon.hpp
    src/RoutingEngine.hpp
    src/Simulation.cpp
    src/Simulation.hpp
    src/SimulationClock.cpp
    src/SimulationClock.hpp
    src/SimulationError.hpp
    src/Stage.cpp
    src/Stage.hpp
    src/StageDescription.hpp
    src/StageManager.cpp
    src/StageManager.hpp
    src/StageSystem.cpp
    src/StageSystem.hpp
    src/StrategicalDesicionSystem.hpp
    src/SurfaceMeshShortestPathRoutingEngine.cpp
    src/SurfaceMeshShortestPathRoutingEngine.hpp
    src/TacticalDecisionSystem.hpp
    src/Timing.cpp
    src/Timing.hpp
    src/Tracing.cpp
    src/Tracing.hpp
    src/UniqueID.hpp
    src/Util.hpp
)

add_subdirectory(src/OperationalModels)
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
        test/TestAABB.cpp
        test/TestAgentView.cpp
        test/TestBasicPrimitiveTests.cpp
        test/TestBoundaryIndex.cpp
        test/TestCommon.hpp
        test/TestCustomModel.cpp
        test/TestGenericAgentFormatter.cpp
        test/TestGeometry.cpp
        test/TestJourney.cpp
        test/TestLineSegment.cpp
        test/TestLocation.cpp
        test/TestNeighborhoodSearch.cpp
        test/TestOperationalDecisionSystem.cpp
        test/TestPoint.cpp
        test/TestProjectedBoundary.cpp
        test/TestRegionSeams.cpp
        test/TestRegionSplit.cpp
        test/TestRegionView.cpp
        test/TestRoutingEngine.cpp
        test/TestSegmentGrid.cpp
        test/TestSimulation.cpp
        test/TestSimulationClock.cpp
        test/TestStage.cpp
        test/TestUniqueID.cpp
        test/TestValidation.cpp
        test/TestWarpDriver.cpp
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
        benchmark/BenchmarkMain.cpp
        benchmark/benchmarkLineSegment.hpp
        benchmark/benchmarkWallQuery.hpp
        benchmark/buildGeometries.hpp
    )

    target_link_libraries(libsimulator-benchmarks PRIVATE
        benchmark::benchmark
        simulator
    )

    target_link_libraries(libsimulator-benchmarks PRIVATE project_options)

endif ()
