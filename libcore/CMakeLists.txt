################################################################################
# Build libcore
################################################################################
set(JPSFIRE_SRC
    JPSfire/A_smoke_sensor/SmokeSensor.cpp
    JPSfire/A_smoke_sensor/SmokeSensor.h
    JPSfire/B_walking_speed/WalkingSpeed.cpp
    JPSfire/B_walking_speed/WalkingSpeed.h
    JPSfire/C_toxicity_analysis/ToxicityAnalysis.cpp
    JPSfire/C_toxicity_analysis/ToxicityAnalysis.h
    JPSfire/C_toxicity_analysis/ToxicityOutputhandler.cpp
    JPSfire/C_toxicity_analysis/ToxicityOutputhandler.h
    JPSfire/generic/FDSMesh.cpp
    JPSfire/generic/FDSMesh.h
    JPSfire/generic/FDSMeshStorage.cpp
    JPSfire/generic/FDSMeshStorage.h
    JPSfire/generic/Knot.cpp
    JPSfire/generic/Knot.h
)
set(AIROUTER_SRC
    routing/ai_router/AIRouter.cpp
    routing/ai_router/AIRouter.h
    routing/ai_router/BrainStorage.cpp
    routing/ai_router/BrainStorage.h
    routing/ai_router/cognitiveMap/associations.cpp
    routing/ai_router/cognitiveMap/associations.h
    routing/ai_router/cognitiveMap/cognitivemap.cpp
    routing/ai_router/cognitiveMap/cognitivemap.h
    routing/ai_router/cognitiveMap/connection.cpp
    routing/ai_router/cognitiveMap/connection.cpp
    routing/ai_router/cognitiveMap/internnavigationnetwork.cpp
    routing/ai_router/cognitiveMap/internnavigationnetwork.h
    routing/ai_router/cognitiveMap/landmark.cpp
    routing/ai_router/cognitiveMap/landmark.h
    routing/ai_router/cognitiveMap/landmarknetwork.cpp
    routing/ai_router/cognitiveMap/landmarknetwork.h
    routing/ai_router/cognitiveMap/region.cpp
    routing/ai_router/cognitiveMap/region.h
    routing/ai_router/Cortex.cpp
    routing/ai_router/Cortex.h
    routing/ai_router/perception/cgalgeometry.cpp
    routing/ai_router/perception/cgalgeometry.h
    routing/ai_router/perception/sign.cpp
    routing/ai_router/perception/sign.h
    routing/ai_router/perception/visibleenvironment.cpp
    routing/ai_router/perception/visibleenvironment.h
    routing/ai_router/perception/visualsystem.cpp
    routing/ai_router/perception/visualsystem.h
)
add_library(core STATIC
    events/Event.cpp
    events/Event.h
    events/EventManager.cpp
    events/EventManager.h
    general/ArgumentParser.cpp
    general/ArgumentParser.h
    general/Compiler.h
    general/Configuration.h
    general/Filesystem.cpp
    general/Filesystem.h
    general/Logger.cpp
    general/Logger.h
    general/Macros.h
    general/randomnumbergenerator.h
    geometry/Building.cpp
    geometry/Building.h
    geometry/Crossing.cpp
    geometry/Crossing.h
    geometry/Goal.cpp
    geometry/Goal.h
    geometry/GoalManager.cpp
    geometry/GoalManager.h
    geometry/Hline.cpp
    geometry/Hline.h
    geometry/Line.cpp
    geometry/Line.h
    geometry/NavLine.cpp
    geometry/NavLine.h
    geometry/Obstacle.cpp
    geometry/Obstacle.h
    geometry/Point.cpp
    geometry/Point.h
    geometry/Room.cpp
    geometry/Room.h
    geometry/SubRoom.cpp
    geometry/SubRoom.h
    geometry/Transition.cpp
    geometry/Transition.h
    geometry/WaitingArea.cpp
    geometry/WaitingArea.h
    geometry/Wall.cpp
    geometry/Wall.h
    IO/GeoFileParser.cpp
    IO/GeoFileParser.h
    IO/IniFileParser.cpp
    IO/IniFileParser.h
    IO/OutputHandler.cpp
    IO/OutputHandler.h
    IO/PedDistributionParser.cpp
    IO/PedDistributionParser.h
    IO/progress_bar.cpp
    IO/Trajectories.cpp
    IO/Trajectories.h
    IO/TraVisToClient.cpp
    IO/TraVisToClient.h
    math/ForceModel.cpp
    math/ForceModel.h
    math/GCFMModel.cpp
    math/GCFMModel.h
    math/GompertzModel.cpp
    math/GompertzModel.h
    math/GradientModel.cpp
    math/GradientModel.h
    math/KrauszModel.cpp
    math/Mathematics.cpp
    math/Mathematics.h
    math/ODESolver.cpp
    math/ODESolver.h
    math/OperationalModel.cpp
    math/OperationalModel.h
    math/VelocityModel.cpp
    math/VelocityModel.h
    mpi/LCGrid.cpp
    mpi/LCGrid.h
    pedestrian/AgentsParameters.cpp
    pedestrian/AgentsParameters.h
    pedestrian/AgentsQueue.cpp
    pedestrian/AgentsQueue.h
    pedestrian/AgentsSource.cpp
    pedestrian/AgentsSource.h
    pedestrian/AgentsSourcesManager.cpp
    pedestrian/AgentsSourcesManager.h
    pedestrian/Ellipse.cpp
    pedestrian/Ellipse.h
    pedestrian/Knowledge.cpp
    pedestrian/Knowledge.h
    pedestrian/PedDistributor.cpp
    pedestrian/PedDistributor.h
    pedestrian/Pedestrian.cpp
    pedestrian/Pedestrian.cpp
    pedestrian/Pedestrian.h
    pedestrian/Pedestrian.h
    pedestrian/StartDistribution.cpp
    pedestrian/StartDistribution.h
    routing/DirectionStrategy.cpp
    routing/DirectionStrategy.h
    routing/ff_router/ffRouter.cpp
    routing/ff_router/ffRouter.h
    routing/ff_router/FloorfieldViaFM.cpp
    routing/ff_router/FloorfieldViaFM.h
    routing/ff_router/UnivFFviaFM.cpp
    routing/ff_router/UnivFFviaFM.h
    routing/global_shortest/AccessPoint.cpp
    routing/global_shortest/AccessPoint.h
    routing/global_shortest/DTriangulation.cpp
    routing/global_shortest/DTriangulation.h
    routing/global_shortest/GlobalRouter.cpp
    routing/global_shortest/GlobalRouter.h
    routing/quickest/QuickestPathRouter.cpp
    routing/quickest/QuickestPathRouter.h
    routing/Router.cpp
    routing/Router.h
    routing/RoutingEngine.cpp
    routing/RoutingEngine.h
    routing/smoke_router/Brain.cpp
    routing/smoke_router/Brain.h
    routing/smoke_router/BrainStorage.cpp
    routing/smoke_router/BrainStorage.h
    routing/smoke_router/cognitiveMap/associations.cpp
    routing/smoke_router/cognitiveMap/associations.h
    routing/smoke_router/cognitiveMap/cogmapoutputhandler.cpp
    routing/smoke_router/cognitiveMap/cogmapoutputhandler.h
    routing/smoke_router/cognitiveMap/cognitivemap.cpp
    routing/smoke_router/cognitiveMap/cognitivemap.h
    routing/smoke_router/cognitiveMap/connection.cpp
    routing/smoke_router/cognitiveMap/connection.cpp
    routing/smoke_router/cognitiveMap/internnavigationnetwork.cpp
    routing/smoke_router/cognitiveMap/internnavigationnetwork.h
    routing/smoke_router/cognitiveMap/landmark.cpp
    routing/smoke_router/cognitiveMap/landmark.h
    routing/smoke_router/cognitiveMap/landmarknetwork.cpp
    routing/smoke_router/cognitiveMap/landmarknetwork.h
    routing/smoke_router/cognitiveMap/region.cpp
    routing/smoke_router/cognitiveMap/region.h
    routing/smoke_router/cognitiveMap/youareherepointer.cpp
    routing/smoke_router/cognitiveMap/youareherepointer.h
    routing/smoke_router/GraphNetwork.cpp
    routing/smoke_router/GraphNetwork.h
    routing/smoke_router/navigation_graph/GraphEdge.cpp
    routing/smoke_router/navigation_graph/GraphEdge.h
    routing/smoke_router/navigation_graph/GraphVertex.cpp
    routing/smoke_router/navigation_graph/GraphVertex.h
    routing/smoke_router/NavigationGraph.cpp
    routing/smoke_router/NavigationGraph.h
    routing/smoke_router/sensor/AbstractSensor.cpp
    routing/smoke_router/sensor/AbstractSensor.h
    routing/smoke_router/sensor/DiscoverDoorsSensor.cpp
    routing/smoke_router/sensor/DiscoverDoorsSensor.h
    routing/smoke_router/sensor/JamSensor.cpp
    routing/smoke_router/sensor/JamSensor.h
    routing/smoke_router/sensor/LastDestinationsSensor.cpp
    routing/smoke_router/sensor/LastDestinationsSensor.h
    routing/smoke_router/sensor/locater.cpp
    routing/smoke_router/sensor/locater.h
    routing/smoke_router/sensor/RoomToFloorSensor.cpp
    routing/smoke_router/sensor/RoomToFloorSensor.h
    routing/smoke_router/sensor/SensorManager.cpp
    routing/smoke_router/sensor/SensorManager.h
    routing/smoke_router/SmokeRouter.cpp
    routing/smoke_router/SmokeRouter.h
    Simulation.cpp
    Simulation.h
    voronoi-boost/VoronoiPositionGenerator.cpp
    voronoi-boost/VoronoiPositionGenerator.h
    $<$<BOOL:${AIROUTER}>:${AIROUTER_SRC}>
    $<$<BOOL:${JPSFIRE}>:${JPSFIRE_SRC}>

)
target_compile_options(core PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(core PUBLIC
    $<$<BOOL:${AIROUTER}>:AIROUTER>
    $<$<BOOL:${JPSFIRE}>:JPSFIRE>
    JPSCORE_VERSION="${PROJECT_VERSION}"
    ${GIT_BUILD_INFO_DEFINES}
)
target_link_libraries(core
    Boost::boost
    $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
    poly2tri
    tinyxml
    visilibity
    CLI11::CLI11
    $<$<BOOL:${AIROUTER}>:CGAL::CGAL>
    $<$<BOOL:${JPSFIRE}>:cnpy>
    fs
    spdlog::spdlog
    fmt::fmt
)
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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
endif()

################################################################################
# Unit tests
################################################################################
if (BUILD_CPPUNIT_TEST)
    find_package(Catch2 REQUIRED)

    add_executable(unittests
        test/catch2/geometry/LineTest.cpp
        test/catch2/geometry/ObstacleTest.cpp
        test/catch2/geometry/PointTest.cpp
        test/catch2/geometry/RoomTest.cpp
        test/catch2/geometry/SubRoomTest.cpp
        test/catch2/Main.cpp
        test/catch2/math/MathematicsTest.cpp
        test/catch2/pedestrian/EllipseTest.cpp
    )

    target_link_libraries(unittests
        Catch2::Catch2
        core
    )

    target_compile_options(unittests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    include(CTest)
    include(ParseAndAddCatchTests)
    set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME Off)
    set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS On)
    ParseAndAddCatchTests(unittests)
endif()
