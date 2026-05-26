################################################################################
# libsimulator/src CMake lists
################################################################################

# central variable that will collect all source files from per-folder cmake files
set(SIMULATOR_SOURCES)

# include per-folder lists (these files append to SIMULATOR_SOURCES)
add_subdirectory(OperationalModels)

# root-level sources (files directly under src)
list(APPEND SIMULATOR_SOURCES
    AABB.cpp
    AABB.hpp
    AgentRemovalSystem.hpp
    Clonable.hpp
    CfgCgal.hpp
    GenericAgent.hpp
    Graph.hpp
    Grid2D.hpp
    HashCombine.hpp
    IteratorPair.hpp
    Journey.cpp
    Journey.hpp
    LineSegment.cpp
    LineSegment.hpp
    Logger.cpp
    Logger.hpp
    Macros.hpp
    Mathematics.cpp
    Mathematics.hpp
    Mesh.cpp
    Mesh.hpp
    NeighborhoodSearch.hpp
    CollisionGeometry.cpp
    CollisionGeometry.hpp
    Ellipse.cpp
    Ellipse.hpp
    GeometryBuilder.cpp
    GeometryBuilder.hpp
    Point.cpp
    Point.hpp
    Polygon.cpp
    Polygon.hpp
    GeometricFunctions.hpp
    GeometrySwitchError.hpp
    TemplateHelper.hpp
    Tracing.cpp
    Tracing.hpp
    UniqueID.hpp
    Util.hpp
    Routing.cpp
    Routing.hpp
    RoutingEngine.cpp
    RoutingEngine.hpp
    Simulation.cpp
    Simulation.hpp
    SimulationClock.cpp
    SimulationClock.hpp
    SimulationError.hpp
    Stage.cpp
    Stage.hpp
    StageDescription.hpp
    StageManager.cpp
    StageManager.hpp
    StageSystem.cpp
    StageSystem.hpp
    StrategicalDesicionSystem.hpp
    TacticalDecisionSystem.hpp
)

list(APPEND SIMULATOR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(simulator STATIC ${SIMULATOR_SOURCES})
set(SIMULATOR_INCLUDE ${SIMULATOR_INCLUDE} PARENT_SCOPE)