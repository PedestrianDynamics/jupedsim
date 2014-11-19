# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF)

cmake_minimum_required(VERSION 2.8)

project(JPSreport)

set(CMAKE_COLOR_MAKEFILE ON)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

#set(EXECUTABLE_OUTPUT_PATH "../")
#INCLUDE_DIRECTORIES("./")
# message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
set(CMAKE_TEST_DIR ${CMAKE_SOURCE_DIR}/Utest)

set(JPSREPORT_MAJOR_VERSION 0)
set(JPSREPORT_MINOR_VERSION 5)
set(JPSREPORT_PATCH_VERSION 0)
set(JPSREPORT_VERSION
  ${JPSREPORT_MAJOR_VERSION}.${JPSREPORT_MINOR_VERSION}.${JPSREPORT_PATCH_VERSION})
message( STATUS "JPSREPORT_VERSION: " ${JPSREPORT_VERSION} )
if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release) 
endif(NOT CMAKE_BUILD_TYPE)
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

if(NOT BUILD_TESTING)
  set(BUILD_TESTING OFF) # test units are not generated.
endif(NOT BUILD_TESTING)
message( STATUS "BUILD_TESTING: " ${BUILD_TESTING} )

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
# To generate a compilation database "compilation_commands.json" for clang_check
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif(NOT CMAKE_EXPORT_COMPILE_COMMANDS)



if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 1)

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
  # add 1 should be magic! http://www.kitware.com/blog/home/post/63
  #math(EXPR PROCESSOR_COUNT "${PROCESSOR_COUNT} + 1") 
  message( STATUS "PROCESSOR_COUNT " ${PROCESSOR_COUNT})
  set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")
endif(PROCESSOR_COUNT)

# test all cpp-files in Utest
if(BUILD_TESTING)
  file(GLOB test_files "${CMAKE_TEST_DIR}/*.cpp")
#  file(GLOB test_py_files "${CMAKE_TEST_DIR}/*/runtest*.py")
  file(GLOB_RECURSE  test_py_files "${CMAKE_TEST_DIR}/*runtest_*.py")
endif(BUILD_TESTING)

set(source_files
  Analysis.cpp
  VoronoiDiagram.cpp
  # VoronoiDiagramGenerator.cpp
  # VoronoiPolygons.cpp
  IO/OutputHandler.cpp
  general/ArgumentParser.cpp
  tinyxml/tinystr.cpp
  tinyxml/tinyxml.cpp
  tinyxml/tinyxmlerror.cpp
  tinyxml/tinyxmlparser.cpp
  geometry/Building.cpp  
  geometry/Line.cpp      
  geometry/Point.cpp    
  geometry/Transition.cpp
  geometry/Crossing.cpp  
  geometry/NavLine.cpp   
  geometry/Room.cpp     
  geometry/Wall.cpp
  geometry/Hline.cpp     
  geometry/Obstacle.cpp  
  geometry/SubRoom.cpp
  geometry/Goal.cpp
)  
set (  header_files
  Analysis.h
  MeasurementArea.h  
  # VoronoiDiagramGenerator.h  
  VoronoiDiagram.h  
  # VoronoiPolygons.h
  IO/OutputHandler.h
  general/ArgumentParser.h
  tinyxml/tinyxml.h
  tinyxml/tinystr.h
  general/Macros.h
  geometry/Building.h  
  geometry/Line.h      
  geometry/Point.h    
  geometry/Transition.h
  geometry/Crossing.h  
  geometry/NavLine.h   
  geometry/Room.h     
  geometry/Wall.h
  geometry/Hline.h     
  geometry/Obstacle.h  
  geometry/SubRoom.h
  geometry/Goal.h  
  )


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Using CLANG++" )
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")



if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Set compiler flags (g++)" )
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)
message(STATUS "Debug flags: " ${CMAKE_CXX_FLAGS_DEBUG} )
message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS_RELEASE} )


if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE MATCHES Debug)
  set(WITH_COVERAGE TRUE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  message(STATUS "Debug flags for coverage: " ${CMAKE_CXX_FLAGS_DEBUG} )
endif(CMAKE_COMPILER_IS_GNUCXX  AND CMAKE_BUILD_TYPE MATCHES Debug)


if((CMAKE_COMPILER_IS_GNUCXX AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.6) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 3.1))
  message(STATUS "Checking for C++11 compiler - available")
else()
  message(STATUS "Checking for C++11 compiler - unavailable")
endif()
# c++-11 flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()


set(Boost_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
set(Boost_LIBRARY_DIR ${CMAKE_SOURCE_DIR})

message( STATUS "Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR} )
message( STATUS "Boost_LIBRARY_DIR: " ${Boost_LIBRARY_DIR} )

#find_package(Boost COMPONENTS system filesystem REQUIRED)
#find_package(Boost  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_library ( core STATIC ${source_files} )
add_executable(
	jpsreport main.cpp
)
target_link_libraries( jpsreport ${Boost_LIBRARIES} )
target_link_libraries( jpsreport core )

if(WIN32)
target_link_libraries (jpsreport wsock32)
endif()


# ----------------------------- cTest ------------------------------------------
if(BUILD_TESTING)
  SET(BUILD_TEST_TIMEOUT 10800   CACHE STRING "Global timeout on all tests (seconds).")
  SET(DART_TESTING_TIMEOUT "${BUILD_TEST_TIMEOUT}" CACHE STRING "" FORCE)
  message(STATUS "Timeout: " ${BUILD_TEST_TIMEOUT} " s"  )
  enable_testing() #adds another build target, which is test for Makefile generators
  include(CTest) #adding Dart support

  #test if code compiles and runs default setting. Takes about 30 seconds
  add_test (jpsreport_compile ${CMAKE_CTEST_COMMAND} 
  --build-and-test "${CMAKE_SOURCE_DIR}" "${EXECUTABLE_OUTPUT_PATH}" #"${CMAKE_BINARY_DIR}"
  --build-generator ${CMAKE_GENERATOR}
  --build-makeprogram ${CMAKE_MAKE_PROGRAM} -j${PROCESSOR_COUNT}
  --build-two-config  
  --build-exe-dir ${EXECUTABLE_OUTPUT_PATH}  # todo wo soll der exe hin?: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  --build-project JPSreport
  --test-command jpsreport --ini=${CMAKE_TEST_DIR}/files/input_UO_180.xml
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
#     message (STATUS "Add python test: " ${test} " | test_src: " ${test_src})
       add_test(NAME ${test} COMMAND ${test_src})
    endforeach(test_src ${test_py_files})
    message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
    # set (test_parameters "${CMAKE_SOURCE_DIR}")
    # add_test(NAME flow  COMMAND "${CMAKE_TEST_DIR}/testflow.py" "${test_parameters}")
  endif(PYTHONINTERP_FOUND)
endif(BUILD_TESTING)


# add_library ( report ${source_files} )
# #----------------------------- ADD TESTS HERE --------------------------
# add_executable(
# 	testGetFilename
# 	Utest/testGetFilename.cpp
# )
# target_link_libraries (testGetFilename report)


# add_executable(
# 	testGetBasename
# 	Utest/testGetBasename.cpp
# )
# target_link_libraries (testGetBasename report)

# # todo: make list of tests and iterate over 
# # add_executable(testSignPos math/Mathematics.cpp Utest/testSign.cpp)
# # #----------------- append to this list more tests ... but first add_executable() ---------------
# set (tests testGetFilename testGetBasename)  #list of tests
# # #--------------------------------------------------------------------
# foreach (test ${tests})
#   add_test(NAME ${test} COMMAND ${test})
# endforeach(test)

