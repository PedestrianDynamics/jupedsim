################################################################################
# Build libsimulator
################################################################################
add_library(simulator STATIC
    src/AABB.cpp
    src/AABB.hpp
    src/AgentRemovalSystem.hpp
    src/Clonable.hpp
    src/CollisionGeometry.cpp
    src/CollisionGeometry.hpp
    src/DTriangulation.cpp
    src/DTriangulation.hpp
    src/Ellipse.cpp
    src/Ellipse.hpp
    src/Enum.hpp
    src/GeneralizedCentrifugalForceModel.cpp
    src/GeneralizedCentrifugalForceModel.hpp
    src/GeneralizedCentrifugalForceModelBuilder.cpp
    src/GeneralizedCentrifugalForceModelBuilder.hpp
    src/GeneralizedCentrifugalForceModelData.hpp
    src/GeneralizedCentrifugalForceModelUpdate.hpp
    src/GenericAgent.hpp
    src/GeometricFunctions.hpp
    src/Geometry.cpp
    src/Geometry.hpp
    src/GeometryBuilder.cpp
    src/GeometryBuilder.hpp
    src/GeometrySwitchError.hpp
    src/Graph.hpp
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
    src/OperationalModel.hpp
    src/OperationalModelUpdate.hpp
    src/Point.cpp
    src/Point.hpp
    src/Polygon.cpp
    src/Polygon.hpp
    src/RoutingEngine.cpp
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
    src/TacticalDecisionSystem.hpp
    src/TemplateHelper.hpp
    src/Tracing.cpp
    src/Tracing.hpp
    src/Triangle.cpp
    src/Triangle.hpp
    src/UniqueID.hpp
    src/Util.hpp
    src/CollisionFreeSpeedModel.cpp
    src/CollisionFreeSpeedModel.hpp
    src/CollisionFreeSpeedModelBuilder.cpp
    src/CollisionFreeSpeedModelBuilder.hpp
    src/CollisionFreeSpeedModelData.hpp
    src/CollisionFreeSpeedModelUpdate.hpp
    src/CollisionFreeSpeedModelV2.cpp
    src/CollisionFreeSpeedModelV2.hpp
    src/CollisionFreeSpeedModelV2Builder.cpp
    src/CollisionFreeSpeedModelV2Builder.hpp
    src/CollisionFreeSpeedModelV2Data.hpp
    src/CollisionFreeSpeedModelV2Update.hpp
)
target_compile_options(simulator PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(simulator PUBLIC
    JPSCORE_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(simulator PUBLIC
    common
    Boost::boost
    poly2tri
    fmt::fmt
    CGAL::CGAL
    build_info
)
target_link_options(simulator PUBLIC
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<BOOL:${BUILD_WITH_ASAN}>>:-fsanitize=address>
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_property(TARGET simulator PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET simulator PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# libsimulator unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(libsimulator-tests
        test/TestAABB.cpp
        test/TestBasicPrimitiveTests.cpp
        test/TestCollisionGeometry.cpp
        test/TestGraph.cpp
        test/TestJourney.cpp
        test/TestLineSegment.cpp
        test/TestNeighborhoodSearch.cpp
        test/TestPoint.cpp
        test/TestSimulationClock.cpp
        test/TestUniqueID.cpp
    )

    target_link_libraries(libsimulator-tests PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        simulator
    )

    target_compile_options(libsimulator-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET libsimulator-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libsimulator-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()

################################################################################
# libjupedsim micro benchmarks
################################################################################
if (BUILD_BENCHMARKS)
    add_executable(libsimulator-benchmarks
        benchmark/BenchmarkMain.cpp
        benchmark/benchmarkLineSegment.hpp
        benchmark/benchmarkCollisionGeometry.hpp
        benchmark/buildGeometries.hpp
    )

    target_link_libraries(libsimulator-benchmarks PRIVATE
        benchmark::benchmark
        simulator
    )

    target_compile_options(libsimulator-benchmarks PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET libsimulator-benchmarks PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libsimulator-benchmarks PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif ()
