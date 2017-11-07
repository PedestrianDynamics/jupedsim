# some options
# -DCMAKE_CXX_COMPILER=clang++   -DCMAKE_C_COMPILER:STRING=clang
# -DCMAKE_BUILD_TYPE=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF)

cmake_minimum_required(VERSION 2.8)

project(JPSreport)

set(CMAKE_COLOR_MAKEFILE ON)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

if (Boost_NO_SYSTEM_PATHS)
  set(Boost_NO_SYSTEM_PATHS ON)
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib")
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_LIBRARY_DIRS})
endif (Boost_NO_SYSTEM_PATHS)

#set(EXECUTABLE_OUTPUT_PATH "../")
#INCLUDE_DIRECTORIES("./")
# message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
set(CMAKE_TEST_DIR ${CMAKE_SOURCE_DIR}/Utest)

set(JPSREPORT_MAJOR_VERSION 0)
set(JPSREPORT_MINOR_VERSION 8)
set(JPSREPORT_PATCH_VERSION 2)
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
  set(BUILD_TESTING OFF) # test units are generated.
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

# test all cpp-files in Utest
if(BUILD_TESTING)
  file(GLOB test_files "${CMAKE_TEST_DIR}/*.cpp")
#  file(GLOB test_py_files "${CMAKE_TEST_DIR}/*/runtest*.py")
  file(GLOB_RECURSE  test_py_files "${CMAKE_TEST_DIR}/*runtest_*.py")
endif(BUILD_TESTING)

set(source_files
  getRSS.c
  Analysis.cpp
  IO/OutputHandler.cpp
  general/ArgumentParser.cpp
  tinyxml/tinystr.cpp
  tinyxml/tinyxml.cpp
  tinyxml/tinyxmlerror.cpp
  tinyxml/tinyxmlparser.cpp
  methods/VoronoiDiagram.cpp
  methods/PedData.cpp
  methods/Method_A.cpp
  methods/Method_B.cpp
  methods/Method_C.cpp
  methods/Method_D.cpp
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
  methods/MeasurementArea.h   
  methods/VoronoiDiagram.h
  methods/PedData.h
  methods/Method_A.h
  methods/Method_B.h
  methods/Method_C.h
  methods/Method_D.h  
  IO/OutputHandler.h
  general/ArgumentParser.h
  general/Macros.h
  tinyxml/tinyxml.h
  tinyxml/tinystr.h
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


#--------------------
if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    message(STATUS "Checking for C++11 compiler - available")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    message(STATUS "Checking for C++0x compiler - available")
  else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no c++11 support. Please use a different C++ compiler.")
  endif()
endif()
#---------------------
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Using Clang " ${CMAKE_CXX_COMPILER_VERSION})
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Using g++ " ${CMAKE_CXX_COMPILER_VERSION})
endif(CMAKE_COMPILER_IS_GNUCXX)

if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Set compiler flags (g++)" )
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-unused-local-typedefs")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wno-unused-local-typedefs")
endif(CMAKE_COMPILER_IS_GNUCXX)
message(STATUS "Debug flags: " ${CMAKE_CXX_FLAGS_DEBUG} )
message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS_RELEASE} -static )


if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE MATCHES Debug)
  set(WITH_COVERAGE TRUE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  message(STATUS "Debug flags for coverage: " ${CMAKE_CXX_FLAGS_DEBUG} )
endif(CMAKE_COMPILER_IS_GNUCXX  AND CMAKE_BUILD_TYPE MATCHES Debug)



#find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Boost  REQUIRED)
message( STATUS "Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR} )
message( STATUS "Boost_LIBRARY_DIR: " ${Boost_LIBRARY_DIR} )

include_directories(${Boost_INCLUDE_DIR})
#link_directories(${Boost_LIBRARY_DIR})

if (WIN32)
find_library (PSAPI Psapi PATH_SUFFIXES "x64")
message (STATUS "PSAPI: ${PSAPI}")
endif()

add_library ( geometrycore STATIC ${source_files} )
add_executable(
	jpsreport main.cpp
)


if(WIN32)
target_link_libraries (jpsreport wsock32)
target_link_libraries( jpsreport ${PSAPI} )
endif()

target_link_libraries( jpsreport geometrycore )



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
    target_link_libraries (${test} geometrycore)
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
  endif(PYTHONINTERP_FOUND)
endif(BUILD_TESTING)
