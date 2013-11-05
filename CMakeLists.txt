# Documentation: Some useful options:
# -D CMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# CMAKE_BUILD_TYPE:STRING=Release (default Debug)

# CMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
# ------------------------- Begin Generic CMake Variable Logging ------------------

# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise 
# this is the top level directory of your build tree 
# MESSAGE( STATUS "CMAKE_BINARY_DIR:         " ${CMAKE_BINARY_DIR} )

# # if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this 
# # is the directory where the compiled or generated files from the current CMakeLists.txt will go to 
# MESSAGE( STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR} )

# # this is the directory, from which cmake was started, i.e. the top level source directory 
# MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )

# # this is the directory where the currently processed CMakeLists.txt is located in 
# MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )

# # contains the full path to the top level directory of your build tree 
# MESSAGE( STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR} )

# # contains the full path to the root of your project source directory,
# # i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command 
# MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )

# # set this variable to specify a common place where CMake should put all executable files
# # (instead of CMAKE_CURRENT_BINARY_DIR)
#MESSAGE( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )

# # set this variable to specify a common place where CMake should put all libraries 
# # (instead of CMAKE_CURRENT_BINARY_DIR)
#MESSAGE( STATUS "LIBRARY_OUTPUT_PATH:     " ${LIBRARY_OUTPUT_PATH} )

# # tell CMake to search first in directories listed in CMAKE_MODULE_PATH
# # when you use FIND_PACKAGE() or INCLUDE()
# MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )

# # this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake) 
# MESSAGE( STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND} )

# # this is the CMake installation directory 
# MESSAGE( STATUS "CMAKE_ROOT: " ${CMAKE_ROOT} )

# # this is the filename including the complete path of the file where this variable is used. 
# MESSAGE( STATUS "CMAKE_CURRENT_LIST_FILE: " ${CMAKE_CURRENT_LIST_FILE} )

# # this is linenumber where the variable is used
# MESSAGE( STATUS "CMAKE_CURRENT_LIST_LINE: " ${CMAKE_CURRENT_LIST_LINE} )

# # this is used when searching for include files e.g. using the FIND_PATH() command.
# MESSAGE( STATUS "CMAKE_INCLUDE_PATH: " ${CMAKE_INCLUDE_PATH} )

# # this is used when searching for libraries e.g. using the FIND_LIBRARY() command.
# MESSAGE( STATUS "CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH} )

# # the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1" 
# MESSAGE( STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )

# # the short system name, e.g. "Linux", "FreeBSD" or "Windows"
# MESSAGE( STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME} )

# # only the version part of CMAKE_SYSTEM 
# MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION} )

# # the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
# MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )

# # is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
# MESSAGE( STATUS "UNIX: " ${UNIX} )

# # is TRUE on Windows, including CygWin 
# MESSAGE( STATUS "WIN32: " ${WIN32} )

# # is TRUE on Apple OS X
# MESSAGE( STATUS "APPLE: " ${APPLE} )

# # is TRUE when using the MinGW compiler in Windows
# MESSAGE( STATUS "MINGW: " ${MINGW} )

# # is TRUE on Windows when using the CygWin version of cmake
# MESSAGE( STATUS "CYGWIN: " ${CYGWIN} )

# # is TRUE on Windows when using a Borland compiler 
# MESSAGE( STATUS "BORLAND: " ${BORLAND} )

# # Microsoft compiler 
# MESSAGE( STATUS "MSVC: " ${MSVC} )
# MESSAGE( STATUS "MSVC_IDE: " ${MSVC_IDE} )
# MESSAGE( STATUS "MSVC60: " ${MSVC60} )
# MESSAGE( STATUS "MSVC70: " ${MSVC70} )
# MESSAGE( STATUS "MSVC71: " ${MSVC71} )
# MESSAGE( STATUS "MSVC80: " ${MSVC80} )
# MESSAGE( STATUS "CMAKE_COMPILER_2005: " ${CMAKE_COMPILER_2005} )


# # set this to true if you don't want to rebuild the object files if the rules have changed, 
# # but not the actual source files or headers (e.g. if you changed the some compiler switches) 
# MESSAGE( STATUS "CMAKE_SKIP_RULE_DEPENDENCY: " ${CMAKE_SKIP_RULE_DEPENDENCY} )

# # since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing. 
# # If you don't like this, set this one to true.
# MESSAGE( STATUS "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY: " ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY} )

# # If set, runtime paths are not added when using shared libraries. Default it is set to OFF
# MESSAGE( STATUS "CMAKE_SKIP_RPATH: " ${CMAKE_SKIP_RPATH} )

# # set this to true if you are using makefiles and want to see the full compile and link 
# # commands instead of only the shortened ones 
#MESSAGE( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

# # this will cause CMake to not put in the rules that re-run CMake. This might be useful if 
# # you want to use the generated build files on another machine. 
# MESSAGE( STATUS "CMAKE_SUPPRESS_REGENERATION: " ${CMAKE_SUPPRESS_REGENERATION} )


# # A simple way to get switches to the compiler is to use ADD_DEFINITIONS(). 
# # But there are also two variables exactly for this purpose: 

# # the compiler flags for compiling C sources 
# MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS} )

# # the compiler flags for compiling C++ sources 
# MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )


# # Choose the type of build.  Example: SET(CMAKE_BUILD_TYPE Debug) 
# MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

# # if this is set to ON, then all libraries are built as shared libraries by default.
# MESSAGE( STATUS "BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS} )

# # the compiler used for C files 
# MESSAGE( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# # the compiler used for C++ files 
# MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )

# # if the compiler is a variant of gcc, this should be set to 1 
# MESSAGE( STATUS "CMAKE_COMPILER_IS_GNUCC: " ${CMAKE_COMPILER_IS_GNUCC} )

# # if the compiler is a variant of g++, this should be set to 1 
# MESSAGE( STATUS "CMAKE_COMPILER_IS_GNUCXX : " ${CMAKE_COMPILER_IS_GNUCXX} )

# # the tools for creating libraries 
# MESSAGE( STATUS "CMAKE_AR: " ${CMAKE_AR} )
# MESSAGE( STATUS "CMAKE_RANLIB: " ${CMAKE_RANLIB} )

#
#MESSAGE( STATUS ": " ${} )

# ------------------------- End of Generic CMake Variable Logging ------------------
cmake_minimum_required(VERSION 2.8  FATAL_ERROR)

project(JPScore)
set(JPSCORE_MAJOR_VERSION 0)
set(JPSCORE_MINOR_VERSION 1)
set(JPSCORE_PATCH_VERSION 0)
set(JPSCORE_VERSION
  ${JPSCORE_MAJOR_VERSION}.${JPSCORE_MINOR_VERSION}.${JPSCORE_PATCH_VERSION})

if(WIN32)
#Set the default compiler to MinGW instead of NMake
#set(-G "MinGW Makefiles")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release) 
endif()
message( STATUS "CMAKE_BUILD_TYPE:     " ${CMAKE_BUILD_TYPE})
#set(LIBRARY_OUTPUT_PATH ./lib/${CMAKE_BUILD_TYPE})
#message( STATUS "LIBRARY_OUTPUT_PATH:     " ${LIBRARY_OUTPUT_PATH} )
set(EXECUTABLE_OUTPUT_PATH "../")
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
	general/ArgumentParser.h  
	general/Macros.h  
	tinyxml/tinyxml.h
	tinyxml/tinystr.h
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
  message(STATUS "Disabling OpenMP support")
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
  message("-- STATUS: CGAL not found. But don't panik ..")
endif(CGAL_FOUND)


if(WIN32)
target_link_libraries (jpscore wsock32)
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message("-- STATUS: Using CLANG++")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  message("-- Set compiler flags (g++)")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
endif()


message("-- Debug flags: " ${CMAKE_CXX_FLAGS_DEBUG})
message("-- Release flags: " ${CMAKE_CXX_FLAGS_RELEASE})

message("**** JPSCORE_VERSION: " ${JPSCORE_VERSION} " ****" )


# if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX) 
# endif()

#cPack
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "FZJ") #required
set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich")
set(CPACK_PACKAGE_NAME "JPScore")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${JPSCORE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JPScore - the simulation core of JuPedSim")
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
  message("packing for non-linux not yet tested... but let's give it a try")
  if ( WIN32 )
    list (APPEND CPACK_GENERATOR "NSIS" )
  elseif (APPLE)
    list (APPEND CPACK_GENERATOR "Bundle" )
  endif ( WIN32 )
endif(UNIX)
INCLUDE(CPack Documentation)

#ADD_CUSTOM_COMMAND(TARGET rebuild
#          POST_BUILD
#          COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}
#          COMMAND ${CMAKE_COMMAND} -E copy ${EXAMPLE_BIN_NAME} ${PROJECT_BINARY_DIR}/.
#)
