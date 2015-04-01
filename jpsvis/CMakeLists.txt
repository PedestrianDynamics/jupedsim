# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF)
#--------------------------------------------------------------------------


# ------------------------- Begin Generic CMake Variable Logging ------------------

# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
MESSAGE( STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME} )

# only the version part of CMAKE_SYSTEM 
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION} )

# the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )

# the compiler used for C files 
MESSAGE( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# the compiler used for C++ files 
MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )
# ------------------------- End of Generic CMake Variable Logging ------------------
cmake_minimum_required(VERSION 2.8)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})

# set project's name
project( JPSvis )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")


#------------------ set important directories --------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
# Debug messages
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

message (STATUS "MODULE_PATH: " ${CMAKE_MODULE_PATH})
set(CMAKE_COLOR_MAKEFILE ON)
set(JPSVIS_MAJOR_VERSION 0)
set(JPSVIS_MINOR_VERSION 5)
set(JPSVIS_PATCH_VERSION 0)
set(JPSVIS_VERSION
  ${JPSVIS_MAJOR_VERSION}.${JPSVIS_MINOR_VERSION}.${JPSVIS_PATCH_VERSION})
message( STATUS "JPSVIS_VERSION: " ${JPSVIS_VERSION} )

# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# these files compile need Cocoa (evtl. Mac specific)
SET(MAC_FIX_SRC src/fix/osx_thread_fix.mm)
SET(MAC_FIX_HDR src/fix/osx_thread_fix.h)

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
  forms/jpsvis.rc
  )  
# all header files that should be treated with moc
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
IF(APPLE)
  SET(SAMPLE_SRCS "${SAMPLE_SRCS} ${MAC_FIX_SRC}")
  SET(SAMPLE_HDR "${SAMPLE_HDR} ${MAC_FIX_HDR}")
ENDIF(APPLE)


# *.ui files
set( SAMPLE_UIS
  forms/settings.ui
  forms/mainwindow.ui  
  )

# and finally a resource file
set( SAMPLE_RCS
  forms/icons.qrc
  )


set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(VTK_USE_HYBRID ON CACHE BOOL "doc" FORCE)
find_package(VTK REQUIRED)

if(VTK_FOUND)
  message(STATUS "found VTK. Version: " ${VTK_VERSION} " |  VTK_DIR: " ${VTK_DIR})
  message(STATUS "VTK_QT_Version: " ${VTK_QT_VERSION})
else(VTK_FOUND)
  message(FATAL_ERROR
    "Cannot build the executable without VTK. Please set the
VTK_DIR")
endif(VTK_FOUND)

include(${VTK_USE_FILE})
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)



# This sets the windows build that will need the special winmain@16 call. Qt provides
#  this for us in the qtmain.lib file. Using this cmake code will ensure we have it
#  linked into our build. Not needed on Unix/OS X/Linux which is why we have the 
#  IF(WIN32) conditional.
IF (WIN32)
  SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32)

#--------------- here comes QT. Version problem (4 or 5?)
#FIND_PACKAGE( Qt4 REQUIRED )
#FIND_PACKAGE(QT REQUIRED)

FIND_PROGRAM(QT_QMAKE_EXECUTABLE NAMES qmake  REQUIRED)
FIND_PACKAGE(Qt COMPONENTS QtXml QT_USE_QTNETWORK QtXmlPatterns REQUIRED)

if("${DESIRED_QT_VERSION}" STREQUAL "")
  set(DESIRED_QT_VERSION "4")
endif()

if( ${DESIRED_QT_VERSION} VERSION_GREATER "4" OR ${DESIRED_QT_VERSION} VERSION_LESS "3")
  message(WARNING "QT version: " ${DESIRED_QT_VERSION} " not supported")
endif()

message(STATUS "DESIRED_QT_VERSION " ${DESIRED_QT_VERSION})
message(STATUS "QT5_INSTALLED: " ${QT3_INSTALLED})
message(STATUS "QT4_INSTALLED: " ${QT4_INSTALLED})
message(STATUS "QT3_INSTALLED: " ${QT3_INSTALLED})

# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
set(QT_USE_QTXML TRUE)  # needed!
set(QT_USE_QTNETWORK TRUE)

# this command will generate rules that will run rcc on all files from SAMPLE_RCS
# in result SAMPLE_RC_SRCS variable will contain paths to files produced by rcc
if( ${QT4_INSTALLED} )
  message(STATUS "Working with Qt4")
  QT4_ADD_RESOURCES( SAMPLE_RC_SRCS ${SAMPLE_RCS} )
  # and finally this will run moc:
  QT4_WRAP_CPP( SAMPLE_MOC_HDRS ${SAMPLE_MOC_HDRS} )
  # this will run uic on .ui files:
  QT4_WRAP_UI( SAMPLE_UI_HDRS ${SAMPLE_UIS} )
  QT4_ADD_RESOURCES(SAMPLE_RESOURCES_RCS ${SAMPLE_RCS})
  # add some useful macros and variables
  # (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
  include( ${QT_USE_FILE} )
elseif( ${QT5_INSTALLED} )
  message(STATUS "Working with Qt5")    
  find_package(Qt5Widgets REQUIRED)
  qt5_wrap_ui(SAMPLE_UI_HDRS ${SAMPLE_UIS})
endif() 

# For Apple set the icns file containing icons

IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE JPSvis.icns) 
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(SAMPLE_SRCS ${SAMPLE_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns)
ENDIF(APPLE)



# here we instruct CMake to build JPSvis executable from all of the source files
ADD_EXECUTABLE( JPSvis  MACOSX_BUNDLE WIN32
  ${SAMPLE_SRCS}
  ${SAMPLE_MOC_HDRS}
  ${SAMPLE_UI_HDRS} 
  ${SAMPLE_RESOURCES_RCS}
  # ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.png
#  JPSvis.icns
  )



# last thing we have to do is to tell CMake what libraries our executable needs,
# luckily FIND_PACKAGE prepared QT_LIBRARIES variable for us:
set(VTK_USE_GUISUPPORT  ON) 
set(VTK_USE_QT  ON)
#set(VTK_USE_QVTK_QTOPENGL ON )

# QVTK is for VTK 5.x. For VTK 6, use ${VTK_LIBRARIES} instead
if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    TARGET_LINK_LIBRARIES( JPSvis ${QT_LIBRARIES} QVTK)
  else()        
    TARGET_LINK_LIBRARIES( JPSvis  ${VTK_LIBRARIES} ${QT_LIBRARIES})
  endif()
else() # this else is not yet tested
  target_link_libraries(JPSvis vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif() 


#=================================================== Compiler flags =================================================
#----------------------------------------------------------------------
# enable as many warnings as possible
set(warning_flags "-Wall -Wextra")
# issue all the warnings demanded by strict iso c and iso c++
set(warning_flags "${warning_flags} -pedantic")
#---------------------------- compiler -------------------------------
# Initialize CXXFLAGS.
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${warning_flags}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${warning_flags} -O0 -g -Wunused-variable -Wunused-parameter -Wunused-function -Wshadow -Wunused")
  #    set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

#--------------------
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
message(STATUS "Checking for C++11 compiler - available")

#---------------------

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
#======================================================================================================================================

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/JPSvis")
IF(APPLE)
  SET(plugin_dest_dir JPSvis.app/Contents/MacOS)
  SET(qtconf_dest_dir JPSvis.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/JPSvis.app")  
ENDIF(APPLE)

IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/JPSvis.exe")
ENDIF(WIN32)


# if (APPLE)
#   set_target_properties(JPSvis PROPERTIES
#     # APPLE_BUNDLE_BUNDLE_NAME "JPSvis"
#     # APPLE_BUNDLE_BUNDLE_VERSION "${JPSVIS_VERSION}"
#     # APPLE_BUNDLE_LONG_VERSION_STRING "version ${JPSVIS_VERSION}"
#     # APPLE_BUNDLE_SHORT_VERSION_STRING "${JPSVIS_VERSION}"
#     # APPLE_BUNDLE_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved."
#     # APPLE_GUI_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved."
#     # APPLE_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org"
#     APPLE_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
# endif ()


#--------------------------------------------------------------------------------
# Install the QtTest application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS JPSvis
  BUNDLE DESTINATION . COMPONENT Runtime
  RUNTIME DESTINATION bin COMPONENT Runtime
  
  )

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)


#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
SET(DIRS ${QT_LIBRARY_DIRS})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
INSTALL(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)


IF(APPLE)
  # set how it shows up in the Info.plist file
  # SET(MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.png)
  SET(MACOSX_BUNDLE_ICON_FILE JPSvis.icns)
  SET(MACOSX_GUI_ICON JPSvis.icns)
  SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved.")
  SET(MACOSX_BUNDLE_BUNDLE_NAME "JPSvis")
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "${JPSVIS_VERSION}")
  SET(MACOSX_BUNDLE_LONG_VERSION_STRING "version ${JPSVIS_VERSION}")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${JPSVIS_VERSION}")
  SET(MACOSX_GUI_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved.")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org")

ENDIF(APPLE)



# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)

