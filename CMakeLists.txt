# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
#--------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8  FATAL_ERROR)

project(JPScore)

set(CMAKE_COLOR_MAKEFILE ON)
set(JPSCORE_MAJOR_VERSION 0)
set(JPSCORE_MINOR_VERSION 1)
set(JPSCORE_PATCH_VERSION 0)
set(JPSCORE_VERSION
  ${JPSCORE_MAJOR_VERSION}.${JPSCORE_MINOR_VERSION}.${JPSCORE_PATCH_VERSION})
message( STATUS "JPSCORE_VERSION: " ${JPSCORE_VERSION} )

# # configure_file (
# #   "${CMAKE_SOURCE_DIR}/Config.h.in"
# #   "${CMAKE_BINARY_DIR}/Config.h"
# # )
# #include_directories ("${CMAKE_BINARY_DIR}")

if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)

  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif(EXISTS "${cpuinfo_file}")

  # Mac:
  # if(APPLE)
  #   find_program(cmd_sys_pro "system_profiler")
  #   if(cmd_sys_pro)
  #     execute_process(COMMAND ${cmd_sys_pro} OUTPUT_VARIABLE info)
  #     string(REGEX REPLACE "^.*Total Number Of Cores: ([0-9]+).*$" "\\1"
  #       PROCESSOR_COUNT "${info}")
  #   endif(cmd_sys_pro)
  # endif(APPLE)

  # Windows:
  if(WIN32)
    set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
  endif(WIN32)
endif(NOT DEFINED PROCESSOR_COUNT)

if(PROCESSOR_COUNT)
  # add 1 should be magic! http://www.kitware.com/blog/home/post/63
  #math(EXPR PROCESSOR_COUNT "${PROCESSOR_COUNT} + 1")
  message( STATUS "PROCESSOR_COUNT: " ${PROCESSOR_COUNT})
  set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")
endif(PROCESSOR_COUNT)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

#------------------ set important directories --------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_TEST_DIR ${CMAKE_SOURCE_DIR}/Utest)
#include_directories( ${LIBRARY_OUTPUT_PATH} )
#add_subdirectory(${LIBRARY_OUTPUT_PATH} )
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )
#add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

#find the correct OpenMP flag
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  message( STATUS "Disabling OpenMP support" )
endif(OPENMP_FOUND)

#find and add the CGAL library
#fixme: complete this section
FIND_PACKAGE(CGAL QUIET)
if(CGAL_FOUND)
  include (${CGAL_USE_FILE})
  INCLUDE_DIRECTORIES(${CGAL_CORE_INCLUDE_DIR})
  add_definitions(-D_CGAL=1)
  target_link_libraries (jpscore ${CGAL_CORE_LIBRARY})
  message (SEND_ERROR "${CGAL_FOUND}")
  message (FATAL_ERROR "${CGAL_CORE_INCLUDE_DIR}")
  message (SEND_ERROR " Erreur: ${CGAL_FOUND}")
  message (SEND_ERROR " Erreur: ${CGAL_USE_FILE}")
else(CGAL_FOUND)
  message( STATUS "CGAL not found. But don't panik .." )
endif(CGAL_FOUND)

# test files: all cpp-files in Utest
file(GLOB test_files "${CMAKE_TEST_DIR}/*.cpp")
set (   source_files
  main.cpp
  Simulation.cpp
  general/ArgumentParser.cpp

  tinyxml/tinystr.cpp
  tinyxml/tinyxml.cpp
  tinyxml/tinyxmlerror.cpp
  tinyxml/tinyxmlparser.cpp

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

  IO/IODispatcher.cpp
  IO/OutputHandler.cpp
  IO/TraVisToClient.cpp

  math/Distribution.cpp
  math/ForceModel.cpp
  math/Mathematics.cpp
  math/ODESolver.cpp

  mpi/LCGrid.cpp

  pedestrian/Ellipse.cpp
  pedestrian/PedDistributor.cpp
  pedestrian/Pedestrian.cpp

  routing/AccessPoint.cpp
  routing/DirectionStrategy.cpp
  routing/DummyRouter.cpp
  routing/GlobalRouter.cpp
  routing/GraphRouter.cpp
  routing/QuickestPathRouter.cpp
  routing/SafestPathRouter.cpp
  routing/Router.cpp
  routing/RoutingEngine.cpp
  routing/NavMesh.cpp
  routing/DTriangulation.cpp
  routing/MeshRouter.cpp
  routing/mesh/Mesh.cpp

  routing/graph/NavLineState.cpp
  routing/graph/RoutingGraph.cpp
  routing/graph/RoutingGraphStorage.cpp

  routing/CognitiveMapRouter.cpp
  routing/cognitive_map/CognitiveMap.cpp
  routing/cognitive_map/NavigationGraph.cpp
  routing/cognitive_map/CognitiveMapStorage.cpp
  routing/cognitive_map/AbstractCognitiveMapCreator.cpp
  routing/cognitive_map/CompleteCognitiveMapCreator.cpp
  routing/cognitive_map/EmptyCognitiveMapCreator.cpp
  routing/cognitive_map/navigation_graph/GraphEdge.cpp
  routing/cognitive_map/navigation_graph/GraphVertex.cpp
  routing/cognitive_map/sensor/RoomToFloorSensor.cpp
  routing/cognitive_map/sensor/SensorManager.cpp
  routing/cognitive_map/sensor/AbstractSensor.cpp
  poly2tri/common/shapes.cpp
  poly2tri/sweep/sweep_context.cpp
  poly2tri/sweep/advancing_front.cpp
  poly2tri/sweep/sweep.cpp
  poly2tri/sweep/cdt.cpp
  )
set (   header_files
  routing/NavMesh.h
  routing/DirectionStrategy.h
  routing/DummyRouter.h
  routing/GlobalRouter.h
  routing/GraphRouter.h
  routing/QuickestPathRouter.h
  routing/SafestPathRouter.h
  routing/Router.h
  routing/RoutingEngine.h
  routing/AccessPoint.h
  routing/DTriangulation.h
  routing/MeshRouter.h
  routing/mesh/Mesh.h
  routing/graph/RoutingGraphStorage.h
  routing/graph/RoutingGraph.h
  routing/graph/NavLineState.h

  routing/CognitiveMapRouter.h
  routing/cognitive_map/CognitiveMap.h
  routing/cognitive_map/NavigationGraph.h
  routing/cognitive_map/CognitiveMapStorage.h
  routing/cognitive_map/AbstractCognitiveMapCreator.h
  routing/cognitive_map/CompleteCognitiveMapCreator.h
  routing/cognitive_map/EmptyCognitiveMapCreator.h
  routing/cognitive_map/navigation_graph/GraphEdge.h
  routing/cognitive_map/navigation_graph/GraphVertex.h
  routing/cognitive_map/sensor/AbstractSensor.h
  routing/cognitive_map/sensor/RoomToFloorSensor.h
  routing/cognitive_map/sensor/SensorManager.h


  pedestrian/Pedestrian.h
  pedestrian/PedDistributor.h
  pedestrian/Ellipse.h

  mpi/LCGrid.h

  tinyxml/tinyxml.h
  tinyxml/tinystr.h

  general/ArgumentParser.h
  general/Macros.h

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

  IO/IODispatcher.h
  IO/OutputHandler.h
  IO/TraVisToClient.h

  math/ForceModel.h
  math/Distribution.h
  math/Mathematics.h
  math/ODESolver.h

  poly2tri/poly2tri.h
  poly2tri/common/shapes.h
  poly2tri/sweep/cdt.h
  poly2tri/common/utils.h
  poly2tri/sweep/sweep_context.h
  poly2tri/sweep/advancing_front.h
  poly2tri/sweep/sweep.h

  )


add_library ( core SHARED ${source_files} )
#add_library(core OBJECT ${source_files} ${header_files})
#Target
add_executable(
  jpscore main.cpp
  )

target_link_libraries(jpscore  core)


if(WIN32)
  target_link_libraries (jpscore core wsock32)
endif(WIN32)


#---------------------------- compiler -------------------------------
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Using CLANG++" )
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Set compiler flags (g++)" )
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall  -std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "In Debug Build")
  set(WITH_COVERAGE TRUE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage -std=c++11")
  set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  message(STATUS "Debug flags for coverage: " ${CMAKE_CXX_FLAGS_DEBUG} )
else(CMAKE_COMPILER_IS_GNUCXX  AND CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS_RELEASE} )
endif(CMAKE_COMPILER_IS_GNUCXX  AND CMAKE_BUILD_TYPE MATCHES Debug)

# ----------------------------- cTest ------------------------------------------
enable_testing() #adds another build target, which is test for Makefile generators
include(CTest) #adding Dart support

#test if code compiles and runs default setting. Takes about 30 seconds
add_test (jpscore_compile ${CMAKE_CTEST_COMMAND}
  --build-and-test "${CMAKE_SOURCE_DIR}" "${EXECUTABLE_OUTPUT_PATH}" #"${CMAKE_BINARY_DIR}"
  --build-generator ${CMAKE_GENERATOR}
  --build-makeprogram ${CMAKE_MAKE_PROGRAM} -j${PROCESSOR_COUNT}
  --build-two-config
  --build-exe-dir ${EXECUTABLE_OUTPUT_PATH}  # todo wo soll der exe hin?: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  --build-project JPScore
  --test-command jpscore --ini=${CMAKE_SOURCE_DIR}/inputfiles/Default/ini.xml
  )

foreach (test_src ${test_files})
  GET_FILENAME_COMPONENT(test ${test_src} NAME_WE)
  add_executable( ${test} ${test_src})
  target_link_libraries (${test} core)
  add_test(NAME ${test} COMMAND ${test})
endforeach(test_src ${test_files})


find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
  message(STATUS "run: " ${CMAKE_SOURCE_DIR}/Utest/testflow.py)
  set (test_parameters "${CMAKE_SOURCE_DIR}")
  add_test(NAME flow  COMMAND "${CMAKE_TEST_DIR}/testflow.py" "${test_parameters}")
  #add_test(testtime  "${CMAKE_SOURCE_DIR}/Utest/run.py")
endif(PYTHONINTERP_FOUND)
