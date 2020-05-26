################################################################################
# Build libcore
################################################################################
set(source_files
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
    src/events/EventHelper.cpp
    src/events/EventManager.cpp
    src/events/TrainEvent.cpp
    src/general/ArgumentParser.cpp
    src/geometry/Building.cpp
    src/geometry/Crossing.cpp
    src/geometry/Goal.cpp
    src/geometry/GoalManager.cpp
    src/geometry/helper/CorrectGeometry.cpp
    src/geometry/Hline.cpp
    src/geometry/Line.cpp
    src/geometry/NavLine.cpp
    src/geometry/Obstacle.cpp
    src/geometry/Point.cpp
    src/geometry/Room.cpp
    src/geometry/SubRoom.cpp
    src/geometry/Transition.cpp
    src/geometry/WaitingArea.cpp
    src/geometry/Wall.cpp
    src/IO/EventFileParser.cpp
    src/IO/GeoFileParser.cpp
    src/IO/IniFileParser.cpp
    src/IO/OutputHandler.cpp
    src/IO/PedDistributionParser.cpp
    src/IO/TrainFileParser.cpp
    src/IO/Trajectories.cpp
    src/JPSfire/A_smoke_sensor/SmokeSensor.cpp
    src/JPSfire/B_walking_speed/WalkingSpeed.cpp
    src/JPSfire/C_toxicity_analysis/ToxicityAnalysis.cpp
    src/JPSfire/C_toxicity_analysis/ToxicityOutputhandler.cpp
    src/JPSfire/generic/FDSMesh.cpp
    src/JPSfire/generic/FDSMeshStorage.cpp
    src/JPSfire/generic/Knot.cpp
    src/math/GCFMModel.cpp
    src/math/Mathematics.cpp
    src/math/OperationalModel.cpp
    src/math/VelocityModel.cpp
    src/neighborhood/NeighborhoodSearch.cpp
    src/pedestrian/AgentsParameters.cpp
    src/pedestrian/AgentsQueue.cpp
    src/pedestrian/AgentsSource.cpp
    src/pedestrian/AgentsSourcesManager.cpp
    src/pedestrian/Ellipse.cpp
    src/pedestrian/Knowledge.cpp
    src/pedestrian/PedDistributor.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/Pedestrian.cpp
    src/pedestrian/StartDistribution.cpp
    src/routing/ff_router/ffRouter.cpp
    src/routing/ff_router/UnivFFviaFM.cpp
    src/routing/global_shortest/AccessPoint.cpp
    src/routing/global_shortest/DTriangulation.cpp
    src/routing/global_shortest/GlobalRouter.cpp
    src/routing/quickest/QuickestPathRouter.cpp
    src/routing/Router.cpp
    src/routing/RoutingEngine.cpp
    src/routing/smoke_router/Brain.cpp
    src/routing/smoke_router/BrainStorage.cpp
    src/routing/smoke_router/cognitiveMap/associations.cpp
    src/routing/smoke_router/cognitiveMap/cogmapoutputhandler.cpp
    src/routing/smoke_router/cognitiveMap/cognitivemap.cpp
    src/routing/smoke_router/cognitiveMap/connection.cpp
    src/routing/smoke_router/cognitiveMap/internnavigationnetwork.cpp
    src/routing/smoke_router/cognitiveMap/landmark.cpp
    src/routing/smoke_router/cognitiveMap/landmarknetwork.cpp
    src/routing/smoke_router/cognitiveMap/region.cpp
    src/routing/smoke_router/cognitiveMap/youareherepointer.cpp
    src/routing/smoke_router/GraphNetwork.cpp
    src/routing/smoke_router/navigation_graph/GraphEdge.cpp
    src/routing/smoke_router/navigation_graph/GraphVertex.cpp
    src/routing/smoke_router/NavigationGraph.cpp
    src/routing/smoke_router/sensor/AbstractSensor.cpp
    src/routing/smoke_router/sensor/DiscoverDoorsSensor.cpp
    src/routing/smoke_router/sensor/JamSensor.cpp
    src/routing/smoke_router/sensor/LastDestinationsSensor.cpp
    src/routing/smoke_router/sensor/locater.cpp
    src/routing/smoke_router/sensor/RoomToFloorSensor.cpp
    src/routing/smoke_router/sensor/SensorManager.cpp
    src/routing/smoke_router/SmokeRouter.cpp
    src/Simulation.cpp
    src/SimulationHelper.cpp
    src/voronoi-boost/VoronoiPositionGenerator.cpp
)
set(header_files
    src/events/DoorEvent.h
    src/events/Event.h
    src/events/EventHelper.h
    src/events/EventManager.h
    src/events/TrainEvent.h
    src/general/ArgumentParser.h
    src/general/Compiler.h
    src/general/Configuration.h
    src/general/Filesystem.h
    src/general/Macros.h
    src/general/randomnumbergenerator.h
    src/geometry/Building.h
    src/geometry/Crossing.h
    src/geometry/Goal.h
    src/geometry/GoalManager.h
    src/geometry/helper/CorrectGeometry.h
    src/geometry/Hline.h
    src/geometry/Line.h
    src/geometry/NavLine.h
    src/geometry/Obstacle.h
    src/geometry/Point.h
    src/geometry/Room.h
    src/geometry/SubRoom.h
    src/geometry/Transition.h
    src/geometry/WaitingArea.h
    src/geometry/Wall.h
    src/IO/EventFileParser.h
    src/IO/GeoFileParser.h
    src/IO/IniFileParser.h
    src/IO/OutputHandler.h
    src/IO/PedDistributionParser.h
    src/IO/TrainFileParser.h
    src/IO/Trajectories.h
    src/JPSfire/A_smoke_sensor/SmokeSensor.h
    src/JPSfire/B_walking_speed/WalkingSpeed.h
    src/JPSfire/C_toxicity_analysis/ToxicityAnalysis.h
    src/JPSfire/C_toxicity_analysis/ToxicityOutputhandler.h
    src/JPSfire/generic/FDSMesh.h
    src/JPSfire/generic/FDSMeshStorage.h
    src/JPSfire/generic/Knot.h
    src/math/GCFMModel.h
    src/math/Mathematics.h
    src/math/OperationalModel.h
    src/math/VelocityModel.h
    src/neighborhood/NeighborhoodSearch.h
    src/neighborhood/Grid2D.h
    src/pedestrian/AgentsParameters.h
    src/pedestrian/AgentsQueue.h
    src/pedestrian/AgentsSource.h
    src/pedestrian/AgentsSourcesManager.h
    src/pedestrian/Ellipse.h
    src/pedestrian/Knowledge.h
    src/pedestrian/PedDistributor.h
    src/pedestrian/Pedestrian.h
    src/pedestrian/StartDistribution.h
    src/routing/ff_router/ffRouter.h
    src/routing/ff_router/mesh/RectGrid.h
    src/routing/ff_router/UnivFFviaFM.h
    src/routing/global_shortest/AccessPoint.h
    src/routing/global_shortest/DTriangulation.h
    src/routing/global_shortest/GlobalRouter.h
    src/routing/quickest/QuickestPathRouter.h
    src/routing/Router.h
    src/routing/RoutingEngine.h
    src/routing/smoke_router/Brain.h
    src/routing/smoke_router/BrainStorage.h
    src/routing/smoke_router/cognitiveMap/associations.h
    src/routing/smoke_router/cognitiveMap/cogmapoutputhandler.h
    src/routing/smoke_router/cognitiveMap/cognitivemap.h
    src/routing/smoke_router/cognitiveMap/connection.cpp
    src/routing/smoke_router/cognitiveMap/internnavigationnetwork.h
    src/routing/smoke_router/cognitiveMap/landmark.h
    src/routing/smoke_router/cognitiveMap/landmarknetwork.h
    src/routing/smoke_router/cognitiveMap/region.h
    src/routing/smoke_router/cognitiveMap/youareherepointer.h
    src/routing/smoke_router/GraphNetwork.h
    src/routing/smoke_router/navigation_graph/GraphEdge.h
    src/routing/smoke_router/navigation_graph/GraphVertex.h
    src/routing/smoke_router/NavigationGraph.h
    src/routing/smoke_router/sensor/AbstractSensor.h
    src/routing/smoke_router/sensor/DiscoverDoorsSensor.h
    src/routing/smoke_router/sensor/JamSensor.h
    src/routing/smoke_router/sensor/LastDestinationsSensor.h
    src/routing/smoke_router/sensor/locater.h
    src/routing/smoke_router/sensor/RoomToFloorSensor.h
    src/routing/smoke_router/sensor/SensorManager.h
    src/routing/smoke_router/SmokeRouter.h
    src/Simulation.h
    src/SimulationHelper.h
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
    $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
    poly2tri
    tinyxml
    visilibity
    CLI11::CLI11
    cnpy
    fs
    spdlog::spdlog
    fmt::fmt
    git-info
    libshared
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
if (BUILD_CPPUNIT_TEST)
    find_package(Catch2 REQUIRED)

    add_executable(unittests
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
            test/catch2/pedestrian/PedestrianTest.cpp
            test/catch2/routing/UnivFFviaFMTest.cpp
            test/catch2/geometry/CorrectGeometryTest.cpp
            )

    target_link_libraries(unittests PRIVATE
        Catch2::Catch2
        core
    )

    target_compile_options(unittests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    set_property(TARGET unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET unittests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

    include(CTest)
    include(ParseAndAddCatchTests)
    set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME Off)
    set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS On)
    ParseAndAddCatchTests(unittests)

    if (CODE_COVERAGE)
        target_code_coverage(unittests)
    endif ()
endif()
