################################################################################
# Build libsimulator
################################################################################
add_library(simulator STATIC
    src/AABB.cpp
    src/AABB.hpp
    src/Agent.cpp
    src/Agent.hpp
    src/AgentExitSystem.cpp
    src/AgentExitSystem.hpp
    src/Area.cpp
    src/Area.hpp
    src/CGALPolygon.cpp
    src/CGALPolygon.hpp
    src/Clonable.hpp
    src/CollisionGeometry.cpp
    src/CollisionGeometry.hpp
    src/ConvexPolygon.cpp
    src/ConvexPolygon.hpp
    src/DTriangulation.cpp
    src/DTriangulation.hpp
    src/Ellipse.cpp
    src/Ellipse.hpp
    src/Enum.hpp
    src/GCFMModel.cpp
    src/GCFMModel.hpp
    src/GCFMModelBuilder.cpp
    src/GCFMModelBuilder.hpp
    src/GeometricFunctions.hpp
    src/Geometry.cpp
    src/Geometry.hpp
    src/GeometryBuilder.cpp
    src/GeometryBuilder.hpp
    src/Graph.hpp
    src/Grid2D.hpp
    src/HashCombine.hpp
    src/IteratorPair.hpp
    src/Journey.cpp
    src/Journey.hpp
    src/Line.cpp
    src/Line.hpp
    src/Logger.cpp
    src/Logger.hpp
    src/Macros.hpp
    src/Mathematics.cpp
    src/Mathematics.hpp
    src/NeighborhoodIterator.cpp
    src/NeighborhoodIterator.hpp
    src/NeighborhoodSearch.cpp
    src/NeighborhoodSearch.hpp
    src/OperationalDecisionSystem.cpp
    src/OperationalDecisionSystem.hpp
    src/OperationalModel.hpp
    src/Point.cpp
    src/Point.hpp
    src/RoutingEngine.cpp
    src/RoutingEngine.hpp
    src/Simulation.cpp
    src/Simulation.hpp
    src/SimulationClock.cpp
    src/SimulationClock.hpp
    src/StrategicalDesicionSystem.cpp
    src/StrategicalDesicionSystem.hpp
    src/TacticalDecisionSystem.cpp
    src/TacticalDecisionSystem.hpp
    src/Triangle.cpp
    src/Triangle.hpp
    src/UniqueID.hpp
    src/VelocityModel.cpp
    src/VelocityModel.hpp
    src/VelocityModelBuilder.cpp
    src/VelocityModelBuilder.hpp
)
target_compile_options(simulator PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(simulator PUBLIC
    JPSCORE_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(simulator PUBLIC
    Boost::boost
    poly2tri
    fmt::fmt
    CGAL::CGAL
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_property(TARGET simulator PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET simulator PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Build jspjupedsim_asan executable - jpsjupedsim with address sanitizer
################################################################################
if(BUILD_WITH_ASAN)
    get_target_property(simulator_source simulator SOURCES)
    get_target_property(simulator_compile_options simulator COMPILE_OPTIONS)
    get_target_property(simulator_compile_definitions simulator COMPILE_DEFINITIONS)
    get_target_property(simulator_link_libraries simulator LINK_LIBRARIES)
    get_target_property(simulator_include_directories simulator INCLUDE_DIRECTORIES)

    add_library(simulator_asan STATIC
        ${simulator_source}
    )

    target_compile_options(simulator_asan PRIVATE
        ${simulator_compile_options}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize=address
    )

    target_compile_definitions(simulator_asan PUBLIC
        ${simulator_compile_definitions}
    )

    target_link_libraries(simulator_asan
        ${simulator_link_libraries}
    )

    target_include_directories(simulator_asan PUBLIC
        ${simulator_include_directories}
    )

    set_property(TARGET simulator_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET simulator_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()

################################################################################
# libsimulator unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(libsimulator-tests
        test/TestBasicPrimitiveTests.cpp
        test/TestCollisionGeometry.cpp
        test/TestGraph.cpp
        test/TestGrid2D.cpp
        test/TestLine.cpp
        test/TestNeighborhoodSearch.cpp
        test/TestSimulationClock.cpp
        test/TestUniqueID.cpp
    )

    target_link_libraries(libsimulator-tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        simulator
    )

    target_compile_options(libsimulator-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET libsimulator-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libsimulator-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
