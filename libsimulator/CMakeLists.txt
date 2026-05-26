################################################################################
# Build libsimulator
################################################################################

# Create an array to store all directories that include headers into.
set(SIMULATOR_INCLUDE)
# The simulator target is created in the src/ subdirectory. This keeps the
# large explicit source list closer to the files and improves maintainability.
add_subdirectory(src)
target_compile_options(simulator PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
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
)
target_include_directories(simulator PUBLIC
    ${SIMULATOR_INCLUDE}
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
        test/TestGenericAgentFormatter.cpp
        test/TestGraph.cpp
        test/TestJourney.cpp
        test/TestLineSegment.cpp
        test/TestMesh.cpp
        test/TestNeighborhoodSearch.cpp
        test/TestPoint.cpp
        test/TestSimulationClock.cpp
        test/TestStage.cpp
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
