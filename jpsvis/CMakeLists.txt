# ------------------------- Begin Generic CMake Variable Logging ------------------
# todo: how to check these:
# sudo apt-get install libvtk-java
# sudo apt-get install python-vtk


# /*C++ comment style not allowed*/


# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise 
# this is the top level directory of your build tree 
MESSAGE( STATUS "CMAKE_BINARY_DIR:         " ${CMAKE_BINARY_DIR} )

# if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this 
# is the directory where the compiled or generated files from the current CMakeLists.txt will go to 
MESSAGE( STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR} )

# this is the directory, from which cmake was started, i.e. the top level source directory 
MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )

# this is the directory where the currently processed CMakeLists.txt is located in 
MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )

# contains the full path to the top level directory of your build tree 
MESSAGE( STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR} )

# contains the full path to the root of your project source directory,
# i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command 
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )

# set this variable to specify a common place where CMake should put all executable files
# (instead of CMAKE_CURRENT_BINARY_DIR)
MESSAGE( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )

# set this variable to specify a common place where CMake should put all libraries 
# (instead of CMAKE_CURRENT_BINARY_DIR)
MESSAGE( STATUS "LIBRARY_OUTPUT_PATH:     " ${LIBRARY_OUTPUT_PATH} )

# tell CMake to search first in directories listed in CMAKE_MODULE_PATH
# when you use FIND_PACKAGE() or INCLUDE()
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )

# this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake) 
MESSAGE( STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND} )

# this is the CMake installation directory 
MESSAGE( STATUS "CMAKE_ROOT: " ${CMAKE_ROOT} )

# this is the filename including the complete path of the file where this variable is used. 
MESSAGE( STATUS "CMAKE_CURRENT_LIST_FILE: " ${CMAKE_CURRENT_LIST_FILE} )

# this is linenumber where the variable is used
MESSAGE( STATUS "CMAKE_CURRENT_LIST_LINE: " ${CMAKE_CURRENT_LIST_LINE} )

# this is used when searching for include files e.g. using the FIND_PATH() command.
MESSAGE( STATUS "CMAKE_INCLUDE_PATH: " ${CMAKE_INCLUDE_PATH} )

# this is used when searching for libraries e.g. using the FIND_LIBRARY() command.
MESSAGE( STATUS "CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH} )

# the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1" 
MESSAGE( STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )

# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
MESSAGE( STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME} )

# only the version part of CMAKE_SYSTEM 
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION} )

# the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )

# is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
MESSAGE( STATUS "UNIX: " ${UNIX} )

# is TRUE on Windows, including CygWin 
MESSAGE( STATUS "WIN32: " ${WIN32} )

# is TRUE on Apple OS X
MESSAGE( STATUS "APPLE: " ${APPLE} )

# is TRUE when using the MinGW compiler in Windows
MESSAGE( STATUS "MINGW: " ${MINGW} )

# is TRUE on Windows when using the CygWin version of cmake
MESSAGE( STATUS "CYGWIN: " ${CYGWIN} )

# is TRUE on Windows when using a Borland compiler 
MESSAGE( STATUS "BORLAND: " ${BORLAND} )

# Microsoft compiler 
MESSAGE( STATUS "MSVC: " ${MSVC} )
MESSAGE( STATUS "MSVC_IDE: " ${MSVC_IDE} )
MESSAGE( STATUS "MSVC60: " ${MSVC60} )
MESSAGE( STATUS "MSVC70: " ${MSVC70} )
MESSAGE( STATUS "MSVC71: " ${MSVC71} )
MESSAGE( STATUS "MSVC80: " ${MSVC80} )
MESSAGE( STATUS "CMAKE_COMPILER_2005: " ${CMAKE_COMPILER_2005} )


# set this to true if you don't want to rebuild the object files if the rules have changed, 
# but not the actual source files or headers (e.g. if you changed the some compiler switches) 
MESSAGE( STATUS "CMAKE_SKIP_RULE_DEPENDENCY: " ${CMAKE_SKIP_RULE_DEPENDENCY} )

# since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing. 
# If you don't like this, set this one to true.
MESSAGE( STATUS "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY: " ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY} )

# If set, runtime paths are not added when using shared libraries. Default it is set to OFF
MESSAGE( STATUS "CMAKE_SKIP_RPATH: " ${CMAKE_SKIP_RPATH} )

# set this to true if you are using makefiles and want to see the full compile and link 
# commands instead of only the shortened ones 
MESSAGE( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

# this will cause CMake to not put in the rules that re-run CMake. This might be useful if 
# you want to use the generated build files on another machine. 
MESSAGE( STATUS "CMAKE_SUPPRESS_REGENERATION: " ${CMAKE_SUPPRESS_REGENERATION} )


# A simple way to get switches to the compiler is to use ADD_DEFINITIONS(). 
# But there are also two variables exactly for this purpose: 

# the compiler flags for compiling C sources 
MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS} )

# the compiler flags for compiling C++ sources 
MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )


# Choose the type of build.  Example: SET(CMAKE_BUILD_TYPE Debug) 
MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

# if this is set to ON, then all libraries are built as shared libraries by default.
MESSAGE( STATUS "BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS} )

# the compiler used for C files 
MESSAGE( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# the compiler used for C++ files 
MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )

# if the compiler is a variant of gcc, this should be set to 1 
MESSAGE( STATUS "CMAKE_COMPILER_IS_GNUCC: " ${CMAKE_COMPILER_IS_GNUCC} )

# if the compiler is a variant of g++, this should be set to 1 
MESSAGE( STATUS "CMAKE_COMPILER_IS_GNUCXX : " ${CMAKE_COMPILER_IS_GNUCXX} )

# the tools for creating libraries 
MESSAGE( STATUS "CMAKE_AR: " ${CMAKE_AR} )
MESSAGE( STATUS "CMAKE_RANLIB: " ${CMAKE_RANLIB} )

#
#MESSAGE( STATUS ": " ${} )

# ------------------------- End of Generic CMake Variable Logging ------------------
cmake_minimum_required(VERSION 2.8)
 # set project's name
project( JPSvis )

  # with SET() command you can change variables or define new ones
  # here we define SAMPLE_SRCS variable that contains a list of all .cpp files
  # note that we don't need \ at the end of line
#ADD_DEFINITIONS( -Wall )
set( SAMPLE_SRCS
  src/geometry/Building.cpp 
  src/geometry/Crossing.cpp 
  src/geometry/Goal.cpp
  src/geometry/Hline.cpp
  src/geometry/Line.cpp
  src/geometry/NavLine.cpp
  src/geometry/Obstacle.cpp
  src/geometry/Point.cpp
  src/geometry/Room.cpp
  src/geometry/SubRoom.cpp
  src/geometry/Transition.cpp
  src/geometry/Wall.cpp
  src/geometry/JPoint.cpp
  src/tinyxml/tinystr.cpp
  src/tinyxml/tinyxml.cpp
  src/tinyxml/tinyxmlerror.cpp
  src/tinyxml/tinyxmlparser.cpp
  src/IO/OutputHandler.cpp
  src/IO/TraVisToClient.cpp
  forms/Settings.cpp
  src/SaxParser.cpp
  src/Debug.cpp
  src/main.cpp
  src/Frame.cpp
  src/InteractorStyle.cpp
  src/Pedestrian.cpp
  src/SimpleVisualisationWindow.cpp
  src/SyncData.cpp
  src/SystemSettings.cpp
  src/ThreadDataTransfert.cpp
  src/ThreadVisualisation.cpp
  src/TimerCallback.cpp
  src/FrameElement.cpp
  src/geometry/LinePlotter2D.cpp
  src/geometry/PointPlotter2D.cpp
  src/geometry/FacilityGeometry.cpp
  src/geometry/LinePlotter.cpp
  src/geometry/PointPlotter.cpp
  src/network/TraVisToServer.cpp
  src/MainWindow.cpp
  src/TrailPlotter.cpp
  )
  
  # another list, this time it includes all header files that should be treated with moc
  set( SAMPLE_HDR
    src/geometry/Building.h
    src/geometry/Crossing.h
    src/geometry/Goal.h
    src/geometry/Hline.h
    src/geometry/Line.h
    src/geometry/NavLine.h
    src/geometry/Obstacle.h
    src/geometry/Point.h
    src/geometry/Room.h
    src/geometry/SubRoom.h
    src/geometry/Transition.h
    src/geometry/Wall.h
    src/geometry/JPoint.h
    src/tinyxml/tinystr.h
    src/tinyxml/tinyxml.h
    src/general/Macros.h
    src/IO/OutputHandler.h
    src/IO/TraVisToClient.h
    forms/Settings.h
    src/SaxParser.h
    src/Debug.h
    src/Frame.h
    src/InteractorStyle.h
    src/Message.h
    src/Pedestrian.h
    src/SimpleVisualisationWindow.h
    src/SyncData.h
    src/SystemSettings.h
    src/ThreadDataTransfert.h
    src/ThreadVisualisation.h
    src/TimerCallback.h
    src/FrameElement.h
    src/extern_var.h
    src/geometry/FacilityGeometry.h
    src/geometry/LinePlotter.h
    src/geometry/PointPlotter.h
    src/geometry/LinePlotter2D.h
    src/geometry/PointPlotter2D.h
    src/network/TraVisToServer.h
    src/MainWindow.h
    src/TrailPlotter.h
)
  
  # some .ui files
  set( SAMPLE_UIS
    forms/settings.ui
    forms/mainwindow.ui  
  )
  
# and finally an resource file
set( SAMPLE_RCS
  forms/icons.qrc
  #  forms/jpsvis.rc
  )
  
  # enable warnings
  #add_definitions( -Wall )
  
  # by default only QtCore and QtGui modules are enabled
  # other modules must be enabled like this:
set( QT_USE_QT3SUPPORT TRUE )   
set( QT_USE_QTXML TRUE )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
  
#set(VTK_USE_HYBRID ON CACHE BOOL "doc" FORCE)
find_package(VTK REQUIRED)
#set(VTK_DIR "/usr/include/vtk-5.8" CACHE PATH "VTK directory override" FORCE)
#set(VTK_INCLUDE_DIRS "/usr/include/vtk-5.8") # CACHE PATH "VTK directory override" FORCE)
#set(VTK_LIBRARY_DIRS "/usr/lib/vtk-5.8/" CACHE PATH "VTK directory override" FORCE)

if(VTK_FOUND)
  message("found VTK. Version:" ${VTK_VERSION}. VTK_DIR: ${VTK_DIR})
else(VTK_FOUND)
  message(FATAL_ERROR
    "Cannot build the executable without VTK. Please set the
VTK_DIR")
endif(VTK_FOUND)

include(${VTK_USE_FILE})
if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  message(WARNING: JPSvis is not tested with vtk v. ${VTK_VERSION})
endif()
#   # Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
#   find_package(Qt5Widgets REQUIRED QUIET)
# else()
find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})
#endif()
  # this command finds Qt4 libraries and sets all required variables
  # note that it's Qt4, not QT4 or qt4
 # FIND_PACKAGE( Qt4 REQUIRED )
  
  # add some useful macros and variables
  # (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
 include( ${QT_USE_FILE} )
  
  # this command will generate rules that will run rcc on all files from SAMPLE_RCS
  # in result SAMPLE_RC_SRCS variable will contain paths to files produced by rcc
  

  QT4_ADD_RESOURCES( SAMPLE_RC_SRCS ${SAMPLE_RCS} )
  
  
  # and finally this will run moc:
  QT4_WRAP_CPP( SAMPLE_MOC_HDRS ${SAMPLE_MOC_HDRS} )
  # this will run uic on .ui files:
  QT4_WRAP_UI( SAMPLE_UI_HDRS ${SAMPLE_UIS} )


  #QT4_WRAP_UI(helloworld_FORMS_HEADERS ${helloworld_FORMS})
  QT4_ADD_RESOURCES(SAMPLE_RESOURCES_RCS ${SAMPLE_RCS})
  
  # we need this to be able to include headers produced by uic in our code
  # (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
  INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#  INCLUDE_DIRECTORIES( ${VTK_INCLUDE_DIRS} "/usr/include/vtk-5.8 ")
  
  # here we instruct CMake to build "sample" executable from all of the source files
  #ADD_EXECUTABLE( sample ${SAMPLE_SRCS} ${SAMPLE_MOC_SRCS} ${SAMPLE_RC_SRCS} ${SAMPLE_UI_HDRS} )
#  ADD_EXECUTABLE( travisto ${SAMPLE_SRCS} ${SAMPLE_MOC_SRCS}  ${SAMPLE_UI_HDRS} )

  ADD_EXECUTABLE( JPSvis ${SAMPLE_SRCS} 
    ${SAMPLE_MOC_HDRS}
    ${SAMPLE_UI_HDRS} 
    ${SAMPLE_RESOURCES_RCS})
  
  # last thing we have to do is to tell CMake what libraries our executable needs,
  # luckily FIND_PACKAGE prepared QT_LIBRARIES variable for us:
set(VTK_USE_GUISUPPORT  ON) 
set(VTK_USE_QT  ON )
set(VTK_USE_QVTK_QTOPENGL ON )

TARGET_LINK_LIBRARIES( JPSvis ${QT_LIBRARIES} QVTK) # QVTK is for VTK 5.x. For VTK 6, use ${VTK_LIBRARIES} instead


if(${VTK_VERSION} VERSION_LESS "6")
  TARGET_LINK_LIBRARIES( JPSvis ${QT_LIBRARIES} QVTK)
else()
  TARGET_LINK_LIBRARIES( JPSvis  ${VTK_LIBRARIES} QVTK)
endif()
