################################################################################
# Build libcore
################################################################################
set(source_files
    src/IO/EventFileParser.cpp
    src/IO/GeoFileParser.cpp
    src/IO/IniFileParser.cpp
    src/IO/NavLineFileParser.cpp
    src/IO/NavLineFileParser.hpp
    src/IO/OutputHandler.cpp
    src/IO/PedDistributionParser.cpp
    src/IO/TrainFileParser.cpp
    src/IO/Trajectories.cpp
    src/Simulation.cpp
    src/SimulationClock.cpp
    src/SimulationHelper.cpp
    src/agent-creation/AgentCreator.cpp
    src/direction/DirectionManager.cpp
    src/direction/DirectionManager.hpp
    src/direction/waiting/WaitingMiddle.cpp
    src/direction/waiting/WaitingMiddle.hpp
    src/direction/waiting/WaitingRandom.cpp
    src/direction/waiting/WaitingRandom.hpp
    src/direction/waiting/WaitingStrategy.cpp
    src/direction/waiting/WaitingStrategy.hpp
    src/direction/walking/DirectionStrategy.cpp
    src/direction/walking/DirectionStrategy.hpp
    src/events/Event.cpp
    src/events/EventManager.cpp
    src/events/EventVisitors.cpp
    src/general/ArgumentParser.cpp
    src/geometry/Building.cpp
    src/geometry/Crossing.cpp
    src/geometry/DTriangulation.cpp
    src/geometry/Goal.cpp
    src/geometry/GoalManager.cpp
    src/geometry/Hline.cpp
    src/geometry/Line.cpp
    src/geometry/Obstacle.cpp
    src/geometry/Point.cpp
    src/geometry/Room.cpp
    src/geometry/SubRoom.cpp
    src/geometry/SubroomType.cpp
    src/geometry/SubroomType.hpp
    src/geometry/Transition.cpp
    src/geometry/WaitingArea.cpp
    src/geometry/Wall.cpp
    src/geometry/helper/CorrectGeometry.cpp
    src/math/GCFMModel.cpp
    src/math/Mathematics.cpp
    src/math/OperationalModel.cpp
    src/math/VelocityModel.cpp
    src/neighborhood/NeighborhoodSearch.cpp
    src/pedestrian/AgentsParameters.cpp
    src/pedestrian/AgentsSource.cpp
    src/pedestrian/AgentsSourcesManager.cpp
    src/pedestrian/Ellipse.cpp
    src/pedestrian/PedDistributor.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/StartDistribution.cpp
    src/routing/RoutingEngine.cpp
    src/routing/RoutingStrategy.cpp
    src/routing/ff_router/UnivFFviaFM.cpp
    src/routing/ff_router/ffRouter.cpp
    src/routing/global_shortest/AccessPoint.cpp
    src/routing/global_shortest/GlobalRouter.cpp
    src/voronoi-boost/VoronoiPositionGenerator.cpp
)

set(header_files
    src/IO/EventFileParser.hpp
    src/IO/GeoFileParser.hpp
    src/IO/IniFileParser.hpp
    src/IO/OutputHandler.hpp
    src/IO/PedDistributionParser.hpp
    src/IO/TrainFileParser.hpp
    src/IO/Trajectories.hpp
    src/Simulation.hpp
    src/SimulationClock.hpp
    src/SimulationHelper.hpp
    src/agent-creation/AgentCreator.hpp
    src/events/Event.hpp
    src/events/EventManager.hpp
    src/events/EventVisitors.hpp
    src/general/ArgumentParser.hpp
    src/general/Compiler.hpp
    src/general/Configuration.hpp
    src/general/Filesystem.hpp
    src/general/Macros.hpp
    src/general/randomnumbergenerator.hpp
    src/geometry/Building.hpp
    src/geometry/Crossing.hpp
    src/geometry/DTriangulation.hpp
    src/geometry/Goal.hpp
    src/geometry/GoalManager.hpp
    src/geometry/Hline.hpp
    src/geometry/Line.hpp
    src/geometry/Obstacle.hpp
    src/geometry/Point.hpp
    src/geometry/Room.hpp
    src/geometry/SubRoom.hpp
    src/geometry/TrainGeometryInterface.hpp
    src/geometry/Transition.hpp
    src/geometry/WaitingArea.hpp
    src/geometry/Wall.hpp
    src/geometry/helper/CorrectGeometry.hpp
    src/math/GCFMModel.hpp
    src/math/Mathematics.hpp
    src/math/OperationalModel.hpp
    src/math/VelocityModel.hpp
    src/neighborhood/Grid2D.hpp
    src/neighborhood/NeighborhoodSearch.hpp
    src/pedestrian/AgentsParameters.hpp
    src/pedestrian/AgentsSource.hpp
    src/pedestrian/AgentsSourcesManager.hpp
    src/pedestrian/Ellipse.hpp
    src/pedestrian/PedDistributor.hpp
    src/pedestrian/Pedestrian.hpp
    src/pedestrian/StartDistribution.hpp
    src/routing/Router.hpp
    src/routing/RoutingEngine.hpp
    src/routing/RoutingStrategy.hpp
    src/routing/ff_router/UnivFFviaFM.hpp
    src/routing/ff_router/ffRouter.hpp
    src/routing/ff_router/mesh/RectGrid.hpp
    src/routing/global_shortest/AccessPoint.hpp
    src/routing/global_shortest/GlobalRouter.hpp
    src/util/UniqueID.hpp
    src/voronoi-boost/VoronoiPositionGenerator.hpp
)

add_library(core STATIC
    ${source_files}
    ${header_files}
)
target_compile_options(core PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(core PUBLIC
    JPSCORE_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(core
    Boost::boost
    poly2tri
    tinyxml
    visilibity
    CLI11::CLI11
    fs
    spdlog::spdlog
    fmt::fmt
    git-info
    shared
)
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set_property(TARGET core PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET core PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# Build jspcore_asan executable - jpscore with address sanitizer
################################################################################
if(BUILD_WITH_ASAN)
    get_target_property(core_source core SOURCES)
    get_target_property(core_compile_options core COMPILE_OPTIONS)
    get_target_property(core_compile_definitions core COMPILE_DEFINITIONS)
    get_target_property(core_link_libraries core LINK_LIBRARIES)
    get_target_property(core_include_directories core INCLUDE_DIRECTORIES)

    add_library(core_asan STATIC
        ${core_source}
    )

    target_compile_options(core_asan PRIVATE
        ${core_compile_options}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize=address
    )

    target_compile_definitions(core_asan PUBLIC
        ${core_compile_definitions}
    )

    target_link_libraries(core_asan
        ${core_link_libraries}
    )

    target_include_directories(core_asan PUBLIC
        ${core_include_directories}
    )

    set_property(TARGET core_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET core_asan PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()

################################################################################
# libcore unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(libcore-tests
        test/TestSimulationClock.cpp
        test/util/TestUniqueID.cpp
        test/neighborhood/TestGrid2D.cpp
    )

    target_link_libraries(libcore-tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        core    
    )
    
    target_compile_options(libcore-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )
    
    set_property(TARGET libcore-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET libcore-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

    add_executable(catch-unittests
        test/catch2/geometry/GeometryHelperTest.cpp
        test/catch2/geometry/LineTest.cpp
        test/catch2/geometry/ObstacleTest.cpp
        test/catch2/geometry/PointTest.cpp
        test/catch2/geometry/RoomTest.cpp
        test/catch2/geometry/SubRoomTest.cpp
        test/catch2/neighborhood/NeighborhoodSearch.cpp
        test/catch2/neighborhood/Grid2D.cpp
        test/catch2/Main.cpp
        test/catch2/math/MathematicsTest.cpp
        test/catch2/pedestrian/EllipseTest.cpp
    )

    target_link_libraries(catch-unittests PRIVATE
        Catch2::Catch2
        core
    )

    target_compile_options(catch-unittests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET catch-unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET catch-unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

    if (CODE_COVERAGE)
        target_code_coverage(catch-unittests)
    endif ()
endif()
