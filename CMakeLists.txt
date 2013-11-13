cmake_minimum_required(VERSION 2.8)

project(JPSreport)

set(CMAKE_COLOR_MAKEFILE ON)

#set(LIBRARY_OUTPUT_PATH ../lib/${CMAKE_BUILD_TYPE})

#set(EXECUTABLE_OUTPUT_PATH "../")
#INCLUDE_DIRECTORIES("./")
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )

set(JPSREPORT_MAJOR_VERSION 0)
set(JPSREPORT_MINOR_VERSION 1)
set(JPSREPORT_PATCH_VERSION 0)
set(JPSREPORT_VERSION
  ${JPSREPORT_MAJOR_VERSION}.${JPSREPORT_MINOR_VERSION}.${JPSREPORT_PATCH_VERSION})
message( STATUS "JPSREPORT_VERSION: " ${JPSREPORT_VERSION} )
if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release) 
endif(NOT CMAKE_BUILD_TYPE)
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}")
message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )
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


file(
	GLOB_RECURSE
	source_files
	Analysis.cpp
	main.cpp
	VoronoiDiagram.cpp
	VoronoiDiagramGenerator.cpp
	VoronoiPolygons.cpp
	IO/IODispatcher.cpp
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

	header_files:
	Analysis.h
	MeasurementArea.h  
	VoronoiDiagramGenerator.h  
	VoronoiDiagram.h  
	VoronoiPolygons.h
	IO/IODispatcher.h
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


add_executable(
	jpsreport
	${source_files}
)

if(WIN32)
target_link_libraries (jpsreport wsock32)
endif()


# ----------------------------- cTest ------------------------------------------
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
  --test-command jpsreport --ini=${CMAKE_SOURCE_DIR}/Testing/input_UO_180.xml
  )

# todo: make list of tests and iterate over 
# add_executable(testSignPos math/Mathematics.cpp Utest/testSign.cpp)
# #----------------- append to this list more tests ... but first add_executable() ---------------
# set (tests testSignPos testSignNeg testPointOnEllipse testEqual)
# #--------------------------------------------------------------------
# foreach (test ${tests})
#   add_test(NAME ${test} COMMAND ${test})
# endforeach(test)

# add_test( NAME timetest
#   COMMAND ${CMAKE_COMMAND}
#   -DTEST_PROG=$<TARGET_FILE:testPointOnEllipse>  
#   -P    ${EXECUTABLE_OUTPUT_PATH}/runtest.cmake
#     )


# find_package(PythonInterp)
# if(PYTHONINTERP_FOUND)
#   message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
#   message(STATUS "run: " ${CMAKE_SOURCE_DIR}/Utest/testflow.py)
#   set (test_parameters "${CMAKE_SOURCE_DIR}")
#   add_test(NAME flow  COMMAND "${CMAKE_SOURCE_DIR}/Utest/testflow.py" "${test_parameters}")
#   #add_test(testtime  "${CMAKE_SOURCE_DIR}/Utest/run.py")
# endif(PYTHONINTERP_FOUND)



# add_test(NAME <name> [CONFIGURATIONS [Debug|Release|...]]
#             [WORKING_DIRECTORY dir]
#             COMMAND <command> [arg1 [arg2 ...]])

