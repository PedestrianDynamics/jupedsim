################################################################################
# Build libcore
################################################################################
add_library(core STATIC
    src/IO/GeoFileParser.cpp
    src/IO/GeoFileParser.hpp
    src/IO/IniFileParser.cpp
    src/IO/IniFileParser.hpp
    src/IO/OutputHandler.cpp
    src/IO/OutputHandler.hpp
    src/IO/PedDistributionParser.cpp
    src/IO/PedDistributionParser.hpp
    src/IO/Trajectories.cpp
    src/IO/Trajectories.hpp
    src/agent-creation/AgentCreator.cpp
    src/agent-creation/AgentCreator.hpp
    src/events/Event.cpp
    src/events/Event.hpp
    src/events/EventManager.cpp
    src/events/EventManager.hpp
    src/events/EventVisitors.cpp
    src/events/EventVisitors.hpp
    src/general/ArgumentParser.cpp
    src/general/ArgumentParser.hpp
    src/general/Compiler.hpp
    src/general/Configuration.hpp
    src/geometry/Building.cpp
    src/geometry/Building.hpp
    src/geometry/Crossing.cpp
    src/geometry/Crossing.hpp
    src/geometry/Goal.cpp
    src/geometry/Goal.hpp
    src/geometry/Hline.cpp
    src/geometry/Hline.hpp
    src/geometry/Obstacle.cpp
    src/geometry/Obstacle.hpp
    src/geometry/Room.cpp
    src/geometry/Room.hpp
    src/geometry/SubRoom.cpp
    src/geometry/SubRoom.hpp
    src/geometry/SubroomType.cpp
    src/geometry/SubroomType.hpp
    src/geometry/Transition.cpp
    src/geometry/Transition.hpp
    src/geometry/WaitingArea.cpp
    src/geometry/WaitingArea.hpp
    src/geometry/Wall.cpp
    src/geometry/Wall.hpp
    src/pedestrian/AgentsParameters.cpp
    src/pedestrian/AgentsParameters.hpp
    src/pedestrian/AgentsSource.cpp
    src/pedestrian/AgentsSource.hpp
    src/pedestrian/AgentsSourcesManager.cpp
    src/pedestrian/AgentsSourcesManager.hpp
    src/pedestrian/PedDistributor.cpp
    src/pedestrian/PedDistributor.hpp
    src/pedestrian/StartDistribution.cpp
    src/pedestrian/StartDistribution.hpp
    src/voronoi-boost/VoronoiPositionGenerator.cpp
    src/voronoi-boost/VoronoiPositionGenerator.hpp
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
    CLI11::CLI11
    fmt::fmt
    git-info
    shared
    simulator
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
        test/TestIniFileParser.cpp
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
endif()
