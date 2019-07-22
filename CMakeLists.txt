# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER=clang++
# -DCMAKE_BUILD_TYPE=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF) for python tests
# -DBUILD_CPPUNIT_TEST=ON (default OFF) for unit tests
# -DUSE_DUAL_ABI=ON (default OFF)  https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html
# -D Boost_NO_SYSTEM_PATHS=true (default false) -D BOOST_ROOT=PATH_where_to_find_boost
# -D AIROUTER=true (default false)
# -D JPSFIRE=ON (default OFF)
#--------------------------------------------------------------------------
################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(JPScore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################################################################
# Optional features
################################################################################
set(JPSFIRE OFF CACHE BOOL "Build with jps fire support")
message(STATUS "JPSFIRE=${JPSFIRE}")

# Options for  jpscore
if(NOT AIROUTER)
    set(AIROUTER false)
else()
    set(AIROUTER true)
endif()

message(STATUS "BUILD_TESTING01: " ${BUILD_TESTING})

set(CMAKE_COLOR_MAKEFILE ON)
set(JPSCORE_MAJOR_VERSION 0)
set(JPSCORE_MINOR_VERSION 8)
set(JPSCORE_PATCH_VERSION 4)
set(JPSCORE_VERSION
${JPSCORE_MAJOR_VERSION}.${JPSCORE_MINOR_VERSION}.${JPSCORE_PATCH_VERSION})
message(STATUS "JPSCORE_VERSION: " ${JPSCORE_VERSION})
add_definitions("-DJPSCORE_VERSION=\"${JPSCORE_VERSION}\"")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
"${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    message(STATUS "Compiling with Intel settings")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -w")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -w -tcheck")
elseif ()
endif ()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

message(STATUS "BUILD_TESTING02: " ${BUILD_TESTING})

if (NOT BUILD_TESTING)
    set(BUILD_TESTING OFF) # test units & python tests are not generated.
endif (NOT BUILD_TESTING)
message(STATUS "BUILD_TESTING: " ${BUILD_TESTING})

if (NOT BUILD_CPPUNIT_TEST)
    set(BUILD_CPPUNIT_TEST OFF)
endif (NOT BUILD_CPPUNIT_TEST)
message(STATUS "BUILD_CPPUNIT_TEST: " ${BUILD_CPPUNIT_TEST})

if (NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    # To generate a compilation database "compilation_commands.json" for clang_check
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif (NOT CMAKE_EXPORT_COMPILE_COMMANDS)

set(CTEST_BUILD_FLAGS "-j")

if (NOT CMAKE_BUILD_TYPE)
    #  set (CMAKE_BUILD_TYPE Release)
    set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
if(NOT USE_DUAL_ABI)
    set (USE_DUAL_ABI FALSE)
endif()
#------------------ set important directories --------------------
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endif()

if(NOT EXECUTABLE_OUTPUT_PATH)
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
endif()
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_TEST_DIR ${CMAKE_SOURCE_DIR}/Utest)
message(STATUS "BUILD_TESTING03: " ${BUILD_TESTING})

# Debug messages
message(STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
message(STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
message(STATUS "CMAKE_TEST_DIR: " ${CMAKE_TEST_DIR})
message(STATUS "RUN TIME " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS "EXECUTABE" ${EXECUTABLE_OUTPUT_PATH})
# ========================== build platform ==========================
message(STATUS "")
message(STATUS "Platform: ")
message(STATUS "    Host: " ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
if (CMAKE_CROSSCOMPILING)
    message(STATUS "    Target: " ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
endif ()
message(STATUS "    CMake: " ${CMAKE_VERSION})
message(STATUS "    CMake generator: " ${CMAKE_GENERATOR})
message(STATUS "    CMake build tool: " ${CMAKE_BUILD_TOOL})
if (MSVC)
    message(STATUS "    MSVC: " ${MSVC_VERSION})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ZLIB_WINAPI)
endif ()
if (CMAKE_GENERATOR MATCHES Xcode)
    message(STATUS "    Xcode: " ${XCODE_VERSION})
endif ()
if (NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
    message(STATUS "    Configuration: " ${CMAKE_BUILD_TYPE})
endif ()
message(STATUS "")

find_package(Git QUIET) # no need for this msg. It comes from cmake.findgit()
find_program(GIT_SCM git DOC "Git version control")
mark_as_advanced(GIT_SCM)
find_file(GITDIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
if (GIT_SCM AND GITDIR)
    # the commit's SHA1, and whether the building workspace was dirty or not
    # describe --match=NeVeRmAtCh --always --tags --abbrev=40 --dirty
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" --no-pager describe --tags --always --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # branch
    execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # the date of the commit
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the subject of the commit
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    # remove # from subject
    string(REGEX REPLACE "\#"
       "" GIT_COMMIT_SUBJECT
       ${GIT_COMMIT_SUBJECT})

else()
    message(STATUS "Not in a git repo")
  endif()

# string(REGEX REPLACE "\#" "" GIT_COMMIT_SUBJECT ${GIT_COMMIT_SUBJECT})

# even if not in a git repository we need to define these
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_SHA1}\"")
add_definitions("-DGIT_COMMIT_DATE=\"${GIT_DATE}\"")
add_definitions("-DGIT_COMMIT_SUBJECT=\"${GIT_COMMIT_SUBJECT}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif (DOXYGEN_FOUND)
#http://stackoverflow.com/questions/1487752/how-do-i-instruct-cmake-to-look-for-libraries-installed-by-macports
if (APPLE)
    # Detect if the "port" command is valid on this system; if so, return full path
    execute_process(COMMAND which port RESULT_VARIABLE DETECT_MACPORTS OUTPUT_VARIABLE MACPORTS_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${DETECT_MACPORTS} EQUAL 0)
        # "/opt/local/bin/port" doesn't have libs, so we get the parent directory
        get_filename_component(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)
        # "/opt/local/bin" doesn't have libs, so we get the parent directory
        get_filename_component(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)
        # "/opt/local" is where MacPorts lives, add `/lib` suffix and link
        link_directories(${MACPORTS_PREFIX}/lib)
        message(STATUS "Macports detected: ${MACPORTS_PREFIX}/lib")
        # SET(CMAKE_SYSTEM_NAME Darwin)
        # # Add MacPorts
        # INCLUDE_DIRECTORIES(/opt/local/include)

        # LINK_DIRECTORIES(/opt/local/lib)

    else ()
        # Recommendation, also add a "brew --prefix" custom command to detect a homebrew build environment
        execute_process(COMMAND brew --prefix RESULT_VARIABLE DETECT_BREW OUTPUT_VARIABLE BREW_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (${DETECT_BREW} EQUAL 0)
            link_directories(${BREW_PREFIX}/lib)
            message(STATUS "Brew detected: ${BREW_PREFIX}")
        endif ()
    endif ()
endif (APPLE)



if (Boost_NO_SYSTEM_PATHS)
    set(Boost_NO_SYSTEM_PATHS ON)
    set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
    set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib")
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_LIBRARY_DIRS})
endif (Boost_NO_SYSTEM_PATHS)
# in case boost is a non-default location
# SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/win32libs/boost")
# SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/win32libs/boost/lib")


# find the correct OpenMP flag
FIND_PACKAGE(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else (OPENMP_FOUND)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
        # somehow find_package(openmp) does not work properly with clang
    else (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Disabling OpenMP support")
    endif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
endif (OPENMP_FOUND)

#statically link all gcc stuffs
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

#boost
if(MSVC)
   find_package(Boost COMPONENTS  QUIET)
else()
   find_package(Boost COMPONENTS  REQUIRED)

endif()

message(STATUS "BUILD_TESTING0: " ${BUILD_TESTING})
if(AIROUTER)
    #CGAL
    find_package(CGAL REQUIRED) # for AI router
endif()

# test all cpp-files in Utest
if (BUILD_TESTING OR BUILD_CPPUNIT_TEST)
        IF (CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    IF (USE_DUAL_ABI)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_USE_CXX11_ABI=0")
    ENDIF(USE_DUAL_ABI)
        set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules/")
    endif (CMAKE_COMPILER_IS_GNUCXX)
    file(GLOB_RECURSE test_files "${CMAKE_TEST_DIR}/*.cpp")
endif (BUILD_TESTING OR BUILD_CPPUNIT_TEST)

if (BUILD_TESTING)
    file(GLOB_RECURSE test_py_files "${CMAKE_TEST_DIR}/*runtest_*.py")
endif (BUILD_TESTING)

# add sources and headers
set(source_files
Simulation.cpp
general/ArgumentParser.cpp
IO/progress_bar.cpp

geometry/Building.cpp
geometry/Line.cpp
geometry/Point.cpp
geometry/Transition.cpp
geometry/Hline.cpp
geometry/Obstacle.cpp
geometry/SubRoom.cpp
geometry/Crossing.cpp
geometry/NavLine.cpp
geometry/Room.cpp
geometry/Wall.cpp
geometry/Goal.cpp
geometry/WaitingArea.cpp

IO/GeoFileParser.cpp
IO/IniFileParser.cpp
IO/IODispatcher.cpp
IO/OutputHandler.cpp
IO/PedDistributionParser.cpp
IO/TraVisToClient.cpp

math/ForceModel.cpp
math/Mathematics.cpp
math/ODESolver.cpp
math/GCFMModel.cpp
math/GompertzModel.cpp
math/GradientModel.cpp
math/VelocityModel.cpp
math/OperationalModel.cpp


mpi/LCGrid.cpp

pedestrian/Ellipse.cpp
pedestrian/PedDistributor.cpp
pedestrian/Pedestrian.cpp
pedestrian/AgentsParameters.cpp
pedestrian/Knowledge.cpp
pedestrian/AgentsQueue.cpp
pedestrian/AgentsSource.cpp
pedestrian/Pedestrian.cpp
pedestrian/AgentsSourcesManager.cpp
pedestrian/StartDistribution.cpp


#voronoi/VoronoiDiagramGenerator.cpp
voronoi-boost/VoronoiPositionGenerator.cpp

#floorfield
routing/ff_router/ffRouter.cpp
#routing/ff_router/LocalFloorfieldViaFM.cpp
routing/ff_router/FloorfieldViaFM.cpp
#routing/ff_router/FFKit.cpp
routing/ff_router/UnivFFviaFM.cpp

#floorfield
routing/ff_router_trips/ffRouterTrips.cpp
routing/ff_router_trips/FloorfieldViaFMTrips.cpp
routing/ff_router_trips/UnivFFviaFMTrips.cpp

        #global_shortest
routing/global_shortest/AccessPoint.cpp
routing/global_shortest/GlobalRouter.cpp
routing/global_shortest/DTriangulation.cpp

#general
routing/DirectionStrategy.cpp
routing/Router.cpp
routing/RoutingEngine.cpp


#quickest
routing/quickest/QuickestPathRouter.cpp

#Smoke router
routing/smoke_router/SmokeRouter.cpp
routing/smoke_router/GraphNetwork.cpp
routing/smoke_router/NavigationGraph.cpp
routing/smoke_router/BrainStorage.cpp
routing/smoke_router/navigation_graph/GraphEdge.cpp
routing/smoke_router/navigation_graph/GraphVertex.cpp
routing/smoke_router/sensor/RoomToFloorSensor.cpp
routing/smoke_router/sensor/SensorManager.cpp
routing/smoke_router/sensor/AbstractSensor.cpp
routing/smoke_router/sensor/DiscoverDoorsSensor.cpp
routing/smoke_router/sensor/JamSensor.cpp
routing/smoke_router/sensor/LastDestinationsSensor.cpp
routing/smoke_router/sensor/locater.cpp
routing/smoke_router/cognitiveMap/associations.cpp
routing/smoke_router/cognitiveMap/connection.cpp
routing/smoke_router/cognitiveMap/cognitivemap.cpp
routing/smoke_router/cognitiveMap/landmark.cpp
routing/smoke_router/cognitiveMap/region.cpp
routing/smoke_router/cognitiveMap/youareherepointer.cpp
routing/smoke_router/cognitiveMap/cogmapoutputhandler.cpp
routing/smoke_router/cognitiveMap/landmarknetwork.cpp
routing/smoke_router/Brain.cpp
routing/smoke_router/cognitiveMap/internnavigationnetwork.cpp


events/EventManager.cpp
events/Event.cpp

geometry/Trips.cpp
routing/trips_router/TripsRouter.cpp

forms/jpscore.rc
math/KrauszModel.cpp geometry/GoalManager.cpp geometry/GoalManager.h)


set(JPSFIRE_SRC
JPSfire/generic/FDSMesh.cpp
JPSfire/generic/Knot.cpp
JPSfire/generic/FDSMeshStorage.cpp
JPSfire/A_smoke_sensor/SmokeSensor.cpp
JPSfire/B_walking_speed/WalkingSpeed.cpp
JPSfire/C_toxicity_analysis/ToxicityAnalysis.cpp
JPSfire/C_toxicity_analysis/ToxicityOutputhandler.cpp
)
set(header_files

#floorfield
routing/ff_router/ffRouter.h
routing/ff_router/RectGrid.h
routing/ff_router/Trial.h
#routing/ff_router/LocalFloorfieldViaFM.h
routing/ff_router/FloorfieldViaFM.h
#routing/ff_router/FFKit.h
routing/ff_router/UnivFFviaFM.h

#floorfield
routing/ff_router_trips/ffRouterTrips.h
routing/ff_router_trips/RectGrid.h
routing/ff_router_trips/Trial.h
routing/ff_router_trips/FloorfieldViaFMTrips.h
routing/ff_router_trips/UnivFFviaFMTrips.h

#general
routing/DirectionStrategy.h
routing/DummyRouter.h
routing/GlobalRouter.h
routing/QuickestPathRouter.h
routing/SafestPathRouter.h
routing/Router.h
routing/RoutingEngine.h

#global_shortest
routing/global_shortest/GlobalRouter.h
routing/global_shortest/AccessPoint.h
routing/global_shortest/DTriangulation.h

#quickest
routing/quickest/QuickestPathRouter.h

#Smoke router
routing/smoke_router/SmokeRouter.h
routing/smoke_router/GraphNetwork.h
routing/smoke_router/NavigationGraph.h
routing/smoke_router/BrainStorage.h
routing/smoke_router/navigation_graph/GraphEdge.h
routing/smoke_router/navigation_graph/GraphVertex.h
routing/smoke_router/sensor/AbstractSensor.h
routing/smoke_router/sensor/RoomToFloorSensor.h
routing/smoke_router/sensor/SensorManager.h
routing/smoke_router/sensor/DiscoverDoorsSensor.h
routing/smoke_router/sensor/JamSensor.h
routing/smoke_router/sensor/LastDestinationsSensor.h
routing/smoke_router/sensor/locater.h
routing/smoke_router/cognitiveMap/associations.h
routing/smoke_router/cognitiveMap/connection.cpp
routing/smoke_router/cognitiveMap/cognitivemap.h
routing/smoke_router/cognitiveMap/landmark.h
routing/smoke_router/cognitiveMap/region.h
routing/smoke_router/cognitiveMap/youareherepointer.h
routing/smoke_router/cognitiveMap/cogmapoutputhandler.h
routing/smoke_router/cognitiveMap/landmarknetwork.h
routing/smoke_router/Brain.h
routing/smoke_router/cognitiveMap/internnavigationnetwork.h

pedestrian/Pedestrian.h
pedestrian/PedDistributor.h
pedestrian/Ellipse.h
pedestrian/AgentsParameters.h
pedestrian/Knowledge.h
pedestrian/AgentsQueue.h
pedestrian/AgentsSource.h
pedestrian/Pedestrian.h
pedestrian/AgentsSourcesManager.h
pedestrian/StartDistribution.h
#voronoi/VoronoiDiagramGenerator.h

voronoi-boost/VoronoiPositionGenerator.h

mpi/LCGrid.h


general/ArgumentParser.h
general/Configuration.h
general/Macros.h
general/randomnumbergenerator.h

geometry/Crossing.h
geometry/NavLine.h
geometry/Room.h
geometry/Building.h
geometry/Wall.h
geometry/Line.h
geometry/Point.h
geometry/Transition.h
geometry/Hline.h
geometry/Obstacle.h
geometry/SubRoom.h
geometry/Goal.h
geometry/WaitingArea.h

IO/GeoFileParser.h
IO/IniFileParser.h
IO/IODispatcher.h
IO/OutputHandler.h
IO/PedDistributionParser.h
IO/TraVisToClient.h

math/ForceModel.h
math/Mathematics.h
math/ODESolver.h
math/GCFMModel.h
math/GompertzModel.h
math/GradientModel.h
math/VelocityModel.h
math/OperationalModel.h

events/EventManager.h
events/Event.h

        geometry/Trips.h
        routing/trips_router/TripsRouter.h

        )

set(JPSFIRE_HDR
JPSfire/generic/FDSMesh.h
JPSfire/generic/Knot.h
JPSfire/generic/FDSMeshStorage.h
JPSfire/A_smoke_sensor/SmokeSensor.h
JPSfire/B_walking_speed/WalkingSpeed.h
JPSfire/C_toxicity_analysis/ToxicityAnalysis.h
JPSfire/C_toxicity_analysis/ToxicityOutputhandler.h

)
SET(AIROUTER_SRC
#AI router
routing/ai_router/AIRouter.cpp
routing/ai_router/BrainStorage.cpp
routing/ai_router/cognitiveMap/associations.cpp
routing/ai_router/cognitiveMap/connection.cpp
routing/ai_router/cognitiveMap/cognitivemap.cpp
routing/ai_router/cognitiveMap/landmark.cpp
routing/ai_router/cognitiveMap/region.cpp
routing/ai_router/cognitiveMap/landmarknetwork.cpp
routing/ai_router/cognitiveMap/internnavigationnetwork.cpp
routing/ai_router/perception/visualsystem.cpp
routing/ai_router/perception/visibleenvironment.cpp
routing/ai_router/perception/cgalgeometry.cpp
routing/ai_router/perception/sign.cpp
routing/ai_router/Cortex.cpp

#AI router trips
#routing/ai_router_trips/AIRouterTrips.cpp
#routing/ai_router_trips/BrainStorage.cpp
#routing/ai_router_trips/cognitiveMap/associations.cpp
#routing/ai_router_trips/cognitiveMap/connection.cpp
#routing/ai_router_trips/cognitiveMap/cognitivemap.cpp
#routing/ai_router_trips/cognitiveMap/landmark.cpp
#routing/ai_router_trips/cognitiveMap/region.cpp
#routing/ai_router_trips/cognitiveMap/landmarknetwork.cpp
#routing/ai_router_trips/cognitiveMap/internnavigationnetwork.cpp
#routing/ai_router_trips/perception/visualsystem.cpp
#routing/ai_router_trips/perception/visibleenvironment.cpp
#routing/ai_router_trips/perception/cgalgeometry.cpp
#routing/ai_router_trips/perception/sign.cpp
#routing/ai_router_trips/Cortex.cpp
)

SET(AIROUTER_HDR
#AI router
routing/ai_router/AIRouter.h
routing/ai_router/BrainStorage.h
routing/ai_router/cognitiveMap/associations.h
routing/ai_router/cognitiveMap/connection.cpp
routing/ai_router/cognitiveMap/cognitivemap.h
routing/ai_router/cognitiveMap/landmark.h
routing/ai_router/cognitiveMap/region.h
routing/ai_router/cognitiveMap/landmarknetwork.h
routing/ai_router/cognitiveMap/internnavigationnetwork.h
routing/ai_router/perception/visualsystem.h
routing/ai_router/perception/visibleenvironment.h
routing/ai_router/perception/cgalgeometry.h
routing/ai_router/perception/sign.h
routing/ai_router/Cortex.h

#AI router trips
routing/ai_router_trips/AIRouterTrips.h
routing/ai_router_trips/BrainStorage.h
routing/ai_router_trips/cognitiveMap/associations.h
routing/ai_router_trips/cognitiveMap/connection.cpp
routing/ai_router_trips/cognitiveMap/cognitivemap.h
routing/ai_router_trips/cognitiveMap/landmark.h
routing/ai_router_trips/cognitiveMap/region.h
routing/ai_router_trips/cognitiveMap/landmarknetwork.h
routing/ai_router_trips/cognitiveMap/internnavigationnetwork.h
routing/ai_router_trips/perception/visualsystem.h
routing/ai_router_trips/perception/visibleenvironment.h
routing/ai_router_trips/perception/cgalgeometry.h
routing/ai_router_trips/perception/sign.h
routing/ai_router_trips/Cortex.h
)

message(STATUS "AIROUTER: ${AIROUTER}")
if(CGAL_FOUND AND AIROUTER)
    SET(source_files ${source_files} ${AIROUTER_SRC})
    SET(header_files ${header_files} ${AIROUTER_HDR})
    message(STATUS "USE AI ROUTER")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAIROUTER")
endif()

if(JPSFIRE)
    SET(source_files ${source_files} ${JPSFIRE_SRC})
    SET(header_files ${header_files} ${JPSFIRE_HDR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DJPSFIRE")
endif()

# ----- add here 3th party
if(JPSFIRE)
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIRS})
    add_library(cnpy STATIC "cnpy/cnpy.cpp")
    target_link_libraries(cnpy ${ZLIB_LIBRARIES})
endif()


# Build included dependencies
add_subdirectory(third-party)


add_library(core STATIC ${source_files})
if(${CMAKE_CXX_COMPILER_ID} MATCHES "^(GNU|Clang)$")
  target_link_libraries(core poly2tri tinyxml visilibity stdc++fs) # link to lib filesystem
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  message(WARNING "TODO: AppleClang requires Xcode 11 and OSX 10.15")
  target_link_libraries(core poly2tri tinyxml visilibity)
endif()


#add_library ( core SHARED ${source_files} )
add_executable(jpscore main.cpp ${CMAKE_SOURCE_DIR}/forms/JPScore.ico)
if (Boost_FOUND)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})

    if(WIN32)
        add_definitions( -DBOOST_ALL_NO_LIB )
        # force all boost libraries to dynamic link (we already disabled
        # autolinking, so I don't know why we need this, but we do!)
        add_definitions( -DBOOST_ALL_DYN_LINK )
        # add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
    endif()
    message(STATUS "BOOST FOUND: " ${Boost_LIB_VERSION})
    message(STATUS "Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
    message(STATUS "Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR})
    message(STATUS "Boost_LIB_VERSION: " ${Boost_LIB_VERSION})
    message(STATUS "Boost_libraries: " ${Boost_LIBRARIES})
    link_directories(${Boost_LIBRARY_DIRS})
    # suppress warnings in boost libraries with attribute SYSTEM
    include_directories(SYSTEM ${Boost_INCLUDE_DIR})

    if(JPSFIRE)
        target_link_libraries(core cnpy)
    else()
        target_link_libraries(core)
    endif()
else()
    message(WARNING did not find BOOST)
endif()

if (GMP_FOUND)
    message(STATUS "GMP FOUND: " ${GMP_FOUND})
    message(STATUS "GMP_LIBRARY: " ${GMP_LIBRARIES})
    message(STATUS "GMP_INCLUDE_DIR: " ${GMP_INCLUDE_DIR})
    include_directories(SYSTEM ${GMP_INCLUDE_DIR})
endif()

if (MPFR_FOUND)
    message(STATUS "MPFR FOUND: " ${MPFR_FOUND})
    message(STATUS "MPFR_LIBRARY: " ${MPFR_LIBRARIES})
    message(STATUS "MPFR_INCLUDE_DIR: " ${MPFR_INCLUDE_DIR})
    include_directories(SYSTEM ${MPFR_INCLUDE_DIR})
    #target_link_libraries(jpscore ${MPFR_LIBRARIES})
endif()


if (CGAL_FOUND)
    message(STATUS "CGAL FOUND: " ${CGAL_FOUND})
    message(STATUS "CGAL_LIBRARY_DIRS: " ${CGAL_LIBRARIES_DIR})
    message(STATUS "CGAL_INCLUDE_DIRS: " ${CGAL_INCLUDE_DIRS})
    #message(STATUS "CGAL_LIB_VERSION: " ${CGAL_LIB_VERSION})
    message(STATUS "CGAL_libraries: " ${CGAL_LIBRARY})
    #link_directories(${CGAL_LIBRARIES_DIR})
    #link_directories(${)
    include_directories(SYSTEM ${CGAL_INCLUDE_DIRS})
    #message(STATUS "CGAL_3rdparty" ${CGAL_3RD_PARTY_LIBRARIES})
    #target_link_libraries(jpscore ${CGAL_LIBRARIES})
endif()


target_link_libraries(jpscore core)

if (WIN32)
    target_link_libraries(jpscore core wsock32)
endif (WIN32)

if(CGAL_FOUND AND GMP_FOUND AND MPFR_FOUND)
    target_link_libraries(jpscore ${GMP_LIBRARIES} ${MPFR_LIBRARIES} ${CGAL_LIBRARY} ${CGAL_3RD_PARTY_LIBRARIES})
endif()

##if (JPS_AS_A_SERVICE)
#find_package(Protobuf REQUIRED)
#set(GRPC_DIR "${CMAKE_SOURCE_DIR}/cmake_modules/")
#find_package(GRPC REQUIRED)
#file(GLOB ProtoFiles "${CMAKE_SOURCE_DIR}/hybrid/*.proto")
#PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
#PROTOBUF_GENERATE_GRPC_CPP(ProtoGRPCSources ProtoGRPCHeaders ${ProtoFiles})
#
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#set(hybrid_source_files
#        ${ProtoSources}
#        ${ProtoGRPCSources}
#        hybrid/JPSserver.cpp
#        hybrid/HybridSimulationManager.cpp
#        hybrid/Latches.cpp
#        hybrid/GeometryFromProtobufLoader.cpp
#        hybrid/PedDistributionFromProtobufLoader.cpp
#        hybrid/IniFromProtobufLoader.cpp
#        hybrid/IniFileWriter.cpp hybrid/SimObserver.cpp hybrid/SimObserver.h)
#
#set(hybrid_header_files
#        ${ProtoHeaders}
#        ${ProtoGRPCHeaders}
#        hybrid/JPSserver.h
#        hybrid/HybridSimulationManager.h
#        hybrid/Latches.h
#        hybrid/GeometryFromProtobufLoader.h
#        hybrid/PedDistributionFromProtobufLoader.h
#        hybrid/IniFromProtobufLoader.h
#        hybrid/IniFileWriter.h)
#add_library(hybrid STATIC ${hybrid_source_files})
#target_link_libraries(hybrid grpc++ grpc protobuf dl)
#target_link_libraries(core hybrid)
##endif (JPS_AS_A_SERVICE)

#----------------------------------------------------------------------
# enable as many warnings as possible
set(warning_flags "-Wall -Wextra")
# issue all the warnings demanded by strict iso c and iso c++
# set(warning_flags "${warning_flags} -pedantic")
# treat warnings as errors
#set(warning_flags "${warning_flags} -Werror")
# definition shadows another
#set(warning_flags "${warning_flags} -Wshadow")
# do not issue warnings for system headers
# set(warning_flags "${warning_flags} -Wno-system-headers")
# overloaded virtual function has a different signature
# set(warning_flags "${warning_flags} -Woverloaded-virtual")
# make string constants const char*
# set(warning_flags "${warning_flags} -Wwrite-strings")
#----------------------------------------------------------------------
#---------------------------- compiler -------------------------------
# Initialize CXXFLAGS.
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warning_flags}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${warning_flags} -Wunused-variable -Wunused-parameter -Wunused-function -Wshadow -Wunused -DTRACE_LOGGING")
#    set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif ()

#--------------------
# if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#     SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
#     SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
#     SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++")
# endif (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

# #if (NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
# include(CheckCXXCompilerFlag)
# CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
# CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

# if (COMPILER_SUPPORTS_CXX11)
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# elseif (COMPILER_SUPPORTS_CXX0X)
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
# else ()
#     message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
# endif ()
#endif ()


if (MSVC)
    message(STATUS "MSVC: " ${MSVC})
    #add_compile_options(cnpy PRIVATE "/W4" "WX-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /WX-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /WX-")
elseif (MSVC_IDE)
    message(STATUS "MSVC_IDE: " ${MSVC_IDE})
elseif(MSVC60)
    message(STATUS "MSVC60: " ${MSVC60})
elseif(MSVC71)
    message(STATUS "MSVC71: " ${MSVC71})
elseif(MSVC80)
    message(STATUS "MSVC80: " ${MSVC80})
elseif(MSVC90)
    message(STATUS "MSVC90: " ${MSVC90})
elseif(MSVC10)
    message(STATUS "MSVC10: " ${MSVC10})
elseif(MSVC12)
    message(STATUS "MSVC12: " ${MSVC12})
elseif(MSVC13)
    message(STATUS "MSVC13: " ${MSVC13})
elseif(MSVC14)
    message(STATUS "MSVC14: " ${MSVC14})
endif()
#---------------------
message(STATUS "C++ Compiler: " ${CMAKE_CXX_COMPILER})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(STATUS "Using Clang " ${CMAKE_CXX_COMPILER_VERSION})
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
if (CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Using g++ " ${CMAKE_CXX_COMPILER_VERSION})
endif (CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    message(STATUS "Debug flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG})
else (CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS} " ,  " ${CMAKE_CXX_FLAGS_RELEASE})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

# PGI ++ Flags
# if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "PGI")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --c++11 -Minform=inform")
# endif ()

if(WIN32)
  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT jpscore)
    message(STATUS "set start project for VS")
  endif()
endif()

# ----------------------------- cTest ------------------------------------------
if (BUILD_TESTING OR BUILD_CPPUNIT_TEST)
  set(counter "0")
  set(BUILD_TEST_TIMEOUT 10800 CACHE STRING "Global timeout on all tests (seconds).")
  set(DART_TESTING_TIMEOUT "${BUILD_TEST_TIMEOUT}" CACHE STRING "" FORCE)
  message(STATUS "Timeout: " ${BUILD_TEST_TIMEOUT} " s")
  enable_testing() # adds another build target, which is test for Makefile generators
  include(CTest) # adding Dart support

  # test if code compiles and runs default setting. Takes about 30 seconds
  if(BUILD_CPPUNIT_TEST)
    foreach (test_src ${test_files})
      MATH(EXPR counter "${counter}+1")
      GET_FILENAME_COMPONENT(test ${test_src} NAME_WE)
      message(STATUS "Add test " ${counter} "  : " ${test})
      add_executable(${test} ${test_src})
      target_link_libraries(${test} core ${Boost_LIBRARIES})
      add_test(NAME ${test} COMMAND ${test})
    endforeach (test_src ${test_files})
  endif()
  # -- new clean geometry tests
  file(GLOB_RECURSE cgeo_inifiles "${CMAKE_TEST_DIR}/test_geometry/clean_geometry/ini*.xml")
  set(ini_counter "0")
  foreach (cgeo-ini ${cgeo_inifiles})
    MATH(EXPR ini_counter "${ini_counter}+1")
    set(testName "test_clean_geometry_${ini_counter}")
    message(STATUS "Add " ${testName})
    add_test(NAME "${testName}" COMMAND jpscore "${cgeo-ini}")
  endforeach()


endif (BUILD_TESTING OR BUILD_CPPUNIT_TEST)

if (BUILD_TESTING)
find_package(PythonInterp)
if (PYTHONINTERP_FOUND)
message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
        foreach (test_src ${test_py_files})
            MATH(EXPR counter "${counter}+1")
            GET_FILENAME_COMPONENT(test ${test_src} NAME_WE)
            message(STATUS "Add py test " ${counter} "  : " ${test})
            # message (STATUS "Add python test: " ${test} " | test_src: " ${PYTHON_EXECUTABLE} " " ${test_src})
            add_test(NAME ${test} COMMAND ${PYTHON_EXECUTABLE} ${test_src})
        endforeach (test_src ${test_py_files})
        # set (test_parameters "${CMAKE_SOURCE_DIR}")
        # add_test(NAME flow  COMMAND "${CMAKE_TEST_DIR}/testflow.py" "${test_parameters}")
    endif (PYTHONINTERP_FOUND)
endif (BUILD_TESTING)


install(TARGETS jpscore
        DESTINATION bin
        COMPONENT applications)
if(WIN32)
    set(CT_DATA_FILE_DIR "demos")
    file(GLOB CT_FILES "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*/*")
    install(FILES ${CT_FILES}
            DESTINATION ${CT_DATA_FILE_DIR}
            COMPONENT demos)
endif()
# ------------- CPACK ------------------------------
IF(UNIX)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    SET( CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org")
    SET(CPACK_DEBIAN_PACKAGE_SUGGESTS, "jpsvis, jpsreport, jpseditor")
    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
    SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()
if(WIN32 AND NOT UNIX)
    message(STATUS "Package generation - Windows")
    set(CPACK_GENERATOR "NSIS")
    find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
    if(NSIS_PATH)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
        message(STATUS "   + NSIS                                 YES ")
    else(NSIS_PATH)
        message(STATUS "   + NSIS                                 NO ")
    endif(NSIS_PATH)

    #CPACK_NSIS_URL_INFO_ABOUT
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)

    message(STATUS "System Dependencies: " ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            DESTINATION sys
            COMPONENT applications)

    # install(FILES "${Boost_INCLUDE_DIR}/../bin/boost_system-vc141-mt-x32-1_67.dll"
    #         "${Boost_INCLUDE_DIR}/../bin/boost_filesystem-vc141-mt-x32-1_67.dll"
    #         "${Boost_INCLUDE_DIR}/../bin/zlib1.dll"
    #         DESTINATION bin
    #         COMPONENT applications)
elseif(APPLE)
    set(DEFAULT_CPACK_GENERATOR "DragNDrop")
endif()

set(CPACK_COMPONENTS_ALL applications)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/LICENSE")
#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\hw.bmp")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/jpscore.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/forms/jpscore.ico")

#install(DIRECTORY "${PROJECT_SOURCE_DIR}/demos/" DESTINATION "demos")


set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_COMPONENTS_ALL applications demos)
set(CPACK_COMPONENT_CTDATA_GROUP "Demos")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "jpscore and dlls")
set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "demo files")
set(CPACK_COMPONENT_DATA_FILES_DESCRIPTION "demo files to get started")
SET(CPACK_PACKAGE_DESCRIPTION "Simulation core of JuPedSim")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The simulation core of JuPedSim, a framework for simulation and analysis of pedestrian dynamics")
SET(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
SET(CPACK_PACKAGE_NAME "jpscore")
SET(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")
SET(CPACK_PACKAGE_VERSION_MAJOR "${JPSCORE_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${JPSCORE_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${JPSCORE_PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${JPSCORE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${JPSCORE_VERSION}")

INCLUDE(CPack)
