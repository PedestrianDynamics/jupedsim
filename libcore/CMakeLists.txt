################################################################################
# Build libcore
################################################################################
set(source_files
    src/IO/EventFileParser.cpp
    src/IO/GeoFileParser.cpp
    src/IO/IniFileParser.cpp
    src/IO/OutputHandler.cpp
    src/IO/PedDistributionParser.cpp
    src/IO/TrainFileParser.cpp
    src/IO/Trajectories.cpp
    src/Simulation.cpp
    src/SimulationClock.cpp
    src/SimulationHelper.cpp
    src/agent-creation/AgentCreator.cpp
    src/direction/DirectionManager.cpp
    src/direction/DirectionManager.h
    src/direction/waiting/WaitingMiddle.cpp
    src/direction/waiting/WaitingMiddle.h
    src/direction/waiting/WaitingRandom.cpp
    src/direction/waiting/WaitingRandom.h
    src/direction/waiting/WaitingStrategy.cpp
    src/direction/waiting/WaitingStrategy.h
    src/direction/walking/DirectionStrategy.cpp
    src/direction/walking/DirectionStrategy.h
    src/events/DoorEvent.cpp
    src/events/Event.cpp
    src/events/EventManager.cpp
    src/events/EventVisitors.cpp
    src/events/OldEvent.cpp
    src/events/OldEventManager.cpp
    src/events/TrainEvent.cpp
    src/general/ArgumentParser.cpp
    src/geometry/Building.cpp
    src/geometry/Crossing.cpp
    src/geometry/DTriangulation.cpp
    src/geometry/Goal.cpp
    src/geometry/GoalManager.cpp
    src/geometry/Hline.cpp
    src/geometry/Line.cpp
    src/geometry/NavLine.cpp
    src/geometry/Obstacle.cpp
    src/geometry/Point.cpp
    src/geometry/Room.cpp
    src/geometry/SubRoom.cpp
    src/geometry/SubroomType.cpp
    src/geometry/SubroomType.h
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
    src/pedestrian/Knowledge.cpp
    src/pedestrian/PedDistributor.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/StartDistribution.cpp
    src/routing/Router.cpp
    src/routing/RoutingEngine.cpp
    src/routing/ff_router/UnivFFviaFM.cpp
    src/routing/ff_router/ffRouter.cpp
    src/routing/global_shortest/AccessPoint.cpp
    src/routing/global_shortest/GlobalRouter.cpp
    src/voronoi-boost/VoronoiPositionGenerator.cpp
)

set(header_files
    src/IO/EventFileParser.h
    src/IO/GeoFileParser.h
    src/IO/IniFileParser.h
    src/IO/OutputHandler.h
    src/IO/PedDistributionParser.h
    src/IO/TrainFileParser.h
    src/IO/Trajectories.h
    src/Simulation.h
    src/SimulationClock.h
    src/SimulationHelper.h
    src/agent-creation/AgentCreator.h
    src/events/DoorEvent.h
    src/events/Event.h
    src/events/EventManager.h
    src/events/EventVisitors.h
    src/events/OldEvent.h
    src/events/OldEventManager.h
    src/events/TrainEvent.h
    src/general/ArgumentParser.h
    src/general/Compiler.h
    src/general/Configuration.h
    src/general/Filesystem.h
    src/general/Macros.h
    src/general/randomnumbergenerator.h
    src/geometry/Building.h
    src/geometry/Crossing.h
    src/geometry/DTriangulation.h
    src/geometry/Goal.h
    src/geometry/GoalManager.h
    src/geometry/Hline.h
    src/geometry/Line.h
    src/geometry/NavLine.h
    src/geometry/Obstacle.h
    src/geometry/Point.h
    src/geometry/Room.h
    src/geometry/SubRoom.h
    src/geometry/TrainGeometryInterface.h
    src/geometry/Transition.h
    src/geometry/WaitingArea.h
    src/geometry/Wall.h
    src/geometry/helper/CorrectGeometry.h
    src/math/GCFMModel.h
    src/math/Mathematics.h
    src/math/OperationalModel.h
    src/math/VelocityModel.h
    src/neighborhood/Grid2D.h
    src/neighborhood/NeighborhoodSearch.h
    src/pedestrian/AgentsParameters.h
    src/pedestrian/AgentsSource.h
    src/pedestrian/AgentsSourcesManager.h
    src/pedestrian/Ellipse.h
    src/pedestrian/Knowledge.h
    src/pedestrian/PedDistributor.h
    src/pedestrian/Pedestrian.h
    src/pedestrian/StartDistribution.h
    src/routing/Router.h
    src/routing/RoutingEngine.h
    src/routing/ff_router/UnivFFviaFM.h
    src/routing/ff_router/ffRouter.h
    src/routing/ff_router/mesh/RectGrid.h
    src/routing/global_shortest/AccessPoint.h
    src/routing/global_shortest/GlobalRouter.h
    src/voronoi-boost/VoronoiPositionGenerator.h
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
        test/catch2/simulation/SimulationHelperTest.cpp
        test/catch2/Main.cpp
        test/catch2/math/MathematicsTest.cpp
        test/catch2/pedestrian/EllipseTest.cpp
        test/catch2/routing/UnivFFviaFMTest.cpp
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
