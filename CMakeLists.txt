# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF)
#--------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8  FATAL_ERROR)

project(JPScore)

set(CMAKE_COLOR_MAKEFILE ON)
set(JPSCORE_MAJOR_VERSION 0)
set(JPSCORE_MINOR_VERSION 6)
set(JPSCORE_PATCH_VERSION 0)
set(JPSCORE_VERSION
  ${JPSCORE_MAJOR_VERSION}.${JPSCORE_MINOR_VERSION}.${JPSCORE_PATCH_VERSION})
message( STATUS "JPSCORE_VERSION: " ${JPSCORE_VERSION} )

if(NOT BUILD_TESTING)
  set(BUILD_TESTING OFF) # test units are not generated.
endif(NOT BUILD_TESTING)
message( STATUS "BUILD_TESTING: " ${BUILD_TESTING} )

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  # To generate a compilation database "compilation_commands.json" for clang_check
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif(NOT CMAKE_EXPORT_COMPILE_COMMANDS)

# Get number of processors. Mac is not supported
if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)

  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif(EXISTS "${cpuinfo_file}")


  # Windows:
  if(WIN32)
    set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
  endif(WIN32)
endif(NOT DEFINED PROCESSOR_COUNT)

if(PROCESSOR_COUNT)
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
# Debug messages
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )

# ========================== build platform ==========================
message(STATUS "")
message(STATUS "Platform: ")
message(STATUS "    Host: "             ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION} ${CMAKE_HOST_SYSTEM_PROCESSOR})
if(CMAKE_CROSSCOMPILING)
  message(STATUS "    Target: "         ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_PROCESSOR})
endif()
message(STATUS "    CMake: "            ${CMAKE_VERSION})
message(STATUS "    CMake generator: "  ${CMAKE_GENERATOR})
message(STATUS "    CMake build tool: " ${CMAKE_BUILD_TOOL})
if(MSVC)
  message(STATUS "    MSVC: "           ${MSVC_VERSION})
endif()
if(CMAKE_GENERATOR MATCHES Xcode)
  message(STATUS "    Xcode: "          ${XCODE_VERSION})
endif()
if(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
  message(STATUS "    Configuration: "  ${CMAKE_BUILD_TYPE})
endif()
message(STATUS "")

# message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
# message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
# message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
# message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

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

# find the correct OpenMP flag
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
else(OPENMP_FOUND)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    # somehow find_package(openmp) does not work properly with clang
  else(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message( STATUS "Disabling OpenMP support" )
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
endif(OPENMP_FOUND)

#statically link all gcc stuffs
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# find and add the CGAL library
#fixme: complete this section
# FIND_PACKAGE(CGAL QUIET)
# if(CGAL_FOUND)
#   include (${CGAL_USE_FILE})
#   INCLUDE_DIRECTORIES(${CGAL_CORE_INCLUDE_DIR})
#   add_definitions(-D_CGAL=1)
#   message (STATUS "${CGAL_FOUND}")
#   message (STATUS "${CGAL_CORE_INCLUDE_DIR}")
#   message (STATUS " Erreur: ${CGAL_FOUND}")
#   message (STATUS " Erreur: ${CGAL_USE_FILE}")
# else(CGAL_FOUND)
#   message( STATUS "CGAL not found." )
# endif(CGAL_FOUND)

# test all cpp-files in Utest
if(BUILD_TESTING)
  file(GLOB test_files "${CMAKE_TEST_DIR}/*.cpp")
  #  file(GLOB test_py_files "${CMAKE_TEST_DIR}/*/runtest*.py")
  file(GLOB_RECURSE  test_py_files "${CMAKE_TEST_DIR}/*runtest_*.py")

  #string(REGEX REPLACE "CMakeFiles/[^;]+;?" "" SRCS "${test_py_files}")

endif(BUILD_TESTING)
# add sources and headers
set (   source_files
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

  math/ForceModel.cpp
  math/Mathematics.cpp
  math/ODESolver.cpp
  math/GPU_GCFMModel.cpp
  math/GCFMModel.cpp
  math/GompertzModel.cpp
  math/OperationalModel.cpp

  mpi/LCGrid.cpp

  pedestrian/Ellipse.cpp
  pedestrian/PedDistributor.cpp
  pedestrian/Pedestrian.cpp
  pedestrian/AgentsParameters.cpp
  pedestrian/Knowledge.cpp

  routing/AccessPoint.cpp
  routing/DirectionStrategy.cpp
  routing/DummyRouter.cpp
  routing/GlobalRouter.cpp
  routing/QuickestPathRouter.cpp
  routing/SafestPathRouter.cpp
  routing/Router.cpp
  routing/RoutingEngine.cpp
  routing/NavMesh.cpp
  routing/DTriangulation.cpp
  routing/MeshRouter.cpp
  routing/mesh/Mesh.cpp

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
  routing/cognitive_map/sensor/DiscoverDoorsSensor.cpp
  routing/cognitive_map/sensor/JamSensor.cpp
  routing/cognitive_map/sensor/LastDestinationsSensor.cpp
  routing/cognitive_map/sensor/SmokeSensor.cpp

  poly2tri/common/shapes.cpp
  poly2tri/sweep/sweep_context.cpp
  poly2tri/sweep/advancing_front.cpp
  poly2tri/sweep/sweep.cpp
  poly2tri/sweep/cdt.cpp
  events/EventManager.cpp
  )

set (   header_files
  routing/NavMesh.h
  routing/DirectionStrategy.h
  routing/DummyRouter.h
  routing/GlobalRouter.h
  routing/QuickestPathRouter.h
  routing/SafestPathRouter.h
  routing/Router.h
  routing/RoutingEngine.h
  routing/AccessPoint.h
  routing/DTriangulation.h
  routing/MeshRouter.h
  routing/mesh/Mesh.h

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
  routing/cognitive_map/sensor/DiscoverDoorsSensor.h
  routing/cognitive_map/sensor/JamSensor.h
  routing/cognitive_map/sensor/LastDestinationsSensor.h
  routing/cognitive_map/sensor/SmokeSensor.h

  pedestrian/Pedestrian.h
  pedestrian/PedDistributor.h
  pedestrian/Ellipse.h
  pedestrian/AgentsParameters.h
  pedestrian/Knowledge.h

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
  math/Mathematics.h
  math/ODESolver.h
  math/GPU_GCFMModel.cpp
  math/GCFMModel.h
  math/GompertzModel.h
  math/OperationalModel.h   

  poly2tri/poly2tri.h
  poly2tri/common/shapes.h
  poly2tri/sweep/cdt.h
  poly2tri/common/utils.h
  poly2tri/sweep/sweep_context.h
  poly2tri/sweep/advancing_front.h
  poly2tri/sweep/sweep.h
  events/EventManager.h
  )


add_library ( core STATIC ${source_files} )
#add_library ( core SHARED ${source_files} )

#Target
add_executable(
  jpscore main.cpp
  )

target_link_libraries(jpscore  core)

if(WIN32)
  target_link_libraries (jpscore core wsock32)
endif(WIN32)

if(CGAL_FOUND)
  target_link_libraries (jpscore ${CGAL_CORE_LIBRARY})
endif(CGAL_FOUND)
#----------------------------------------------------------------------
# enable as many warnings as possible
# set(warning_flags "-Wall -Wextra")
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
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${warning_flags}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${warning_flags} -O0 -g -Wunused-variable -Wunused-parameter -Wunused-function -Wshadow -Wunused -DTRACE_LOGGING")
  #    set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

#--------------------
if(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if( COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
endif()
message(STATUS "MSVC: " ${MSVC})
message(STATUS "MSVC_IDE: " ${MSVC_IDE})
message(STATUS "MSVC60: " ${MSVC60})
message(STATUS "MSVC71: " ${MSVC71})
message(STATUS "MSVC80: " ${MSVC80})
message(STATUS "MSVC90: " ${MSVC90})
message(STATUS "MSVC10: " ${MSVC10})
message(STATUS "MSVC12: " ${MSVC12})
message(STATUS "MSVC13: " ${MSVC13})

#---------------------
message(STATUS "C++ Compiler: "           ${CMAKE_CXX_COMPILER})
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Using Clang " ${CMAKE_CXX_COMPILER_VERSION})
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Using g++ " ${CMAKE_CXX_COMPILER_VERSION})
endif(CMAKE_COMPILER_IS_GNUCXX)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  message(STATUS "Debug flags: "  ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG} )
else(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS} " ,  " ${CMAKE_CXX_FLAGS_RELEASE} )
endif(CMAKE_BUILD_TYPE MATCHES Debug)

# PGI ++ Flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "PGI")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --c++11 -Minform=inform")	
endif()


# ----------------------------- cTest ------------------------------------------
if(BUILD_TESTING)
  SET(BUILD_TEST_TIMEOUT 10800   CACHE STRING "Global timeout on all tests (seconds).")
  SET(DART_TESTING_TIMEOUT "${BUILD_TEST_TIMEOUT}" CACHE STRING "" FORCE)
  message(STATUS "Timeout: " ${BUILD_TEST_TIMEOUT} " s"  )
  enable_testing() # adds another build target, which is test for Makefile generators
  include(CTest) # adding Dart support

  # test if code compiles and runs default setting. Takes about 30 seconds
  add_test (jpscore_compile ${CMAKE_CTEST_COMMAND}
    --build-and-test "${CMAKE_SOURCE_DIR}"  "${CMAKE_SOURCE_DIR}" #"${EXECUTABLE_OUTPUT_PATH}" #"${CMAKE_BINARY_DIR}"
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
    foreach (test_src ${test_py_files})
      GET_FILENAME_COMPONENT(test ${test_src} NAME_WE)
      message (STATUS "Add python test: " ${test} " | test_src: " ${PYTHON_EXECUTABLE} " " ${test_src})
      add_test(NAME ${test} COMMAND ${PYTHON_EXECUTABLE} ${test_src})
    endforeach(test_src ${test_py_files})
    message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
    # set (test_parameters "${CMAKE_SOURCE_DIR}")
    # add_test(NAME flow  COMMAND "${CMAKE_TEST_DIR}/testflow.py" "${test_parameters}")
  endif(PYTHONINTERP_FOUND)
endif(BUILD_TESTING)
