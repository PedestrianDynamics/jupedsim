# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
#--------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8  FATAL_ERROR)
project(JPScore)
set(CMAKE_COLOR_MAKEFILE ON)
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )

set(JPSCORE_MAJOR_VERSION 0)
set(JPSCORE_MINOR_VERSION 1)
set(JPSCORE_PATCH_VERSION 0)
set(JPSCORE_VERSION
  ${JPSCORE_MAJOR_VERSION}.${JPSCORE_MINOR_VERSION}.${JPSCORE_PATCH_VERSION})
message( STATUS "JPSCORE_VERSION: " ${JPSCORE_VERSION} )

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
  message( STATUS "PROCESSOR_COUNT " ${PROCESSOR_COUNT})
  set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")
endif(PROCESSOR_COUNT)

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release) 
endif(NOT CMAKE_BUILD_TYPE)
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

#------------------ set important directories --------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

file(
	GLOB_RECURSE
	source_files
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
	routing/Router.cpp
	routing/RoutingEngine.cpp
	routing/NavMesh.cpp
	routing/DTriangulation.cpp    
	routing/MeshRouter.cpp    
	routing/mesh/Mesh.cpp    
	
	routing/graph/NavLineState.cpp  
	routing/graph/RoutingGraph.cpp  
	routing/graph/RoutingGraphStorage.cpp  


	poly2tri/common/shapes.cpp          
	poly2tri/sweep/sweep_context.cpp
	poly2tri/sweep/advancing_front.cpp  
	poly2tri/sweep/sweep.cpp
	poly2tri/sweep/cdt.cpp

	header_files:
	routing/NavMesh.ch
	routing/DirectionStrategy.h    
	routing/DummyRouter.h    
	routing/GlobalRouter.h    
	routing/GraphRouter.h    
	routing/QuickestPathRouter.h    
	routing/Router.h
	routing/RoutingEngine.h
	routing/AccessPoint.h    
	routing/DTriangulation.h 
	routing/MeshRouter.h    
	routing/mesh/Mesh.h     
	routing/graph/RoutingGraphStorage.h
	routing/graph/RoutingGraph.h  
	routing/graph/NavLineState.h  

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

# foreach(next_ITEM ${source_files})
#    message(STATUS "next item: ${next_ITEM}")
# endforeach(next_ITEM ${sourc_files})
#Target
add_executable(
  jpscore
  ${source_files}
)

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
	#message (SEND_ERROR "${CGAL_FOUND}")
	#message (FATAL_ERROR "${CGAL_CORE_INCLUDE_DIR}")
	#message (SEND_ERROR " Erreur: ${CGAL_FOUND}")
	#message (SEND_ERROR " Erreur: ${CGAL_USE_FILE}")
else(CGAL_FOUND)
  message( STATUS "CGAL not found. But don't panik .." )
endif(CGAL_FOUND)


if(WIN32)
target_link_libraries (jpscore wsock32)
endif(WIN32)


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

#--------------------------- cPack ---------------------------------------------
set(CPACK_PACKAGE_NAME "JPScore")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JPScore - the simulation core of JuPedSim")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "FZJ") #required
set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/../LICENSE.txt")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${JPSCORE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.txt")
set(CPACK_PACKAGE_VERSION ${JPSCORE_VERSION})
SET (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET (CPACK_DEBIAN_PACKAGE_SECTION "developement")
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libvtk5.8, libvtk5.8-qt4, libqt4-dev, libopenmpi-dev")

#linux
if(UNIX)
  SET(CPACK_GENERATOR "DEB")
  #set(CPACK_PACKAGING_INSTALL_PREFIX  "bin" CACHE PATH "Where you want to install your package")
  mark_as_advanced(CPACK_PACKAGING_INSTALL_PREFIX)
  set(CPACK_STRIP_FILES "jpscore") #remove unnecessary information from executable files
  set(CPACK_SOURCE_STRIP_FILES "")
  #INCLUDE(UseDebian)
  #IF(DEBIAN_FOUND)
  #  ADD_DEBIAN_TARGETS(jpscore)
 #ENDIF(DEBIAN_FOUND)
  ## RPM variables - use rpm -i <soft> --nodeps if you want to skeep the dependences search
  #set(CPACK_RPM_PACKAGE_REQUIRES  "qt4 >= 4.7.4, openmpi >= 1.4.3, cmake >= 2.8, OpenSceneGraph >= 3.0.0")
  #set(CPACK_RPM_PACKAGE_DEBUG     1)
  INSTALL(TARGETS jpscore DESTINATION bin)
else(UNIX)
  message(" packing for non-linux not yet tested... but let's give it a try" )
  if ( WIN32 )
    list (APPEND CPACK_GENERATOR "NSIS" )
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
    SET(CPACK_NSIS_DISPLAY_NAME "JPScore")
    SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
  else ( WIN32 )
    list (APPEND CPACK_GENERATOR "Bundle" )
  endif ( WIN32 )
endif(UNIX)
#INCLUDE(CPack Documentation)
#include(CheckCXXSourceRuns)
#include(CheckCXXSourceCompiles)
set (CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}")   

# ----------------------------- cTest ------------------------------------------
enable_testing() #adds another build target, which is test for Makefile generators
include(CTest) #adding Dart support

#test if code compiles and runs default setting. Takes about 30 seconds

add_test (jpscore_compile_and_test ${CMAKE_CTEST_COMMAND} 
  --build-and-test "${CMAKE_SOURCE_DIR}" "${EXECUTABLE_OUTPUT_PATH}" #"${CMAKE_BINARY_DIR}"
  --build-generator ${CMAKE_GENERATOR}
  --build-makeprogram ${CMAKE_MAKE_PROGRAM}
  --build-two-config  
  --build-exe-dir ${EXECUTABLE_OUTPUT_PATH}  # todo wo soll der exe hin?: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  --build-project JPScore
  --test-command jpscore --ini=${CMAKE_SOURCE_DIR}/inputfiles/Default/ini.xml
  )

## todo: make list of tests and iterate over 
add_executable(testSignPos math/Mathematics.cpp Utest/testSign.cpp)
add_executable(testSignNeg math/Mathematics.cpp Utest/testSign1.cpp)
add_executable(testPointOnEllipse pedestrian/Ellipse.cpp geometry/Point.cpp math/Mathematics.cpp geometry/Line.cpp Utest/testPointOnEllipse.cpp)
#----------------- append to this list more tests ... but first add_executable() ---------------
set (tests testSignPos testSignNeg testPointOnEllipse)
#--------------------------------------------------------------------
foreach (test ${tests})
  add_test(NAME ${test} COMMAND ${test})
endforeach(test)

# add_test( NAME timetest
#   COMMAND ${CMAKE_COMMAND}
#   -DTEST_PROG=$<TARGET_FILE:testPointOnEllipse>  
#   -P    ${EXECUTABLE_OUTPUT_PATH}/runtest.cmake
#     )


find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
  message(STATUS "run: " ${CMAKE_SOURCE_DIR}/Utest/testflow.py)
  add_test(testtime  "${CMAKE_SOURCE_DIR}/Utest/testflow.py")
  #add_test(testtime  "${CMAKE_SOURCE_DIR}/Utest/run.py")
endif(PYTHONINTERP_FOUND)



# add_test(NAME <name> [CONFIGURATIONS [Debug|Release|...]]
#             [WORKING_DIRECTORY dir]
#             COMMAND <command> [arg1 [arg2 ...]])


