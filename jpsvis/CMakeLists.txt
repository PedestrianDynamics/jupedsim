# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF)
# MARK_AS_ADVANCED (BUILD_TESTING CMAKE_BUILD_TYPE=Debug)
#windows flags: -DDESIRED_QT_VERSION=5 -DCMAKE_PREFIX_PATH=D:\Qt32bit\Qt5.4.1\5.4\msvc2013 -DCMAKE_PREFIX_PATH=VTK.0.0\VTK-7.0.0\build -G "Visual Studio 12 2013"
#--------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)
project( jpsvis )

MESSAGE( STATUS "System ........................................ ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_VERSION}, ${CMAKE_SYSTEM_PROCESSOR})" )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_COLOR_MAKEFILE ON)
set(JPSVIS_MAJOR_VERSION 0)
set(JPSVIS_MINOR_VERSION 8)
set(JPSVIS_PATCH_VERSION 1)
set(JPSVIS_VERSION
  ${JPSVIS_MAJOR_VERSION}.${JPSVIS_MINOR_VERSION}.${JPSVIS_PATCH_VERSION})


#------------------ set important directories --------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
message( STATUS "Generating  ................................... ${PROJECT_NAME} (${JPSVIS_VERSION})")

# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(README_FILE "${CMAKE_SOURCE_DIR}/README.md")
if(APPLE AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
	set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH}) #"/Applications")
endif()

# these files compile need Cocoa (evtl. Mac specific)
SET(MAC_FIX_SRC src/fix/osx_thread_fix.mm) #.mm
SET(MAC_FIX_HDR src/fix/osx_thread_fix.h) #.h
set( SRCS
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
  src/geometry/GeometryFactory.cpp
  src/network/TraVisToServer.cpp
  src/MainWindow.cpp
  src/TrailPlotter.cpp
  forms/jpsvis.rc
  )  
# all header files that should be treated with moc
set( HDR
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
  src/geometry/GeometryFactory.h
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
  SET(SRCS ${SRCS} ${MAC_FIX_SRC})
  SET(HDR ${HDR} ${MAC_FIX_HDR})
ENDIF(APPLE)



# *.ui files
set( UIS
  forms/settings.ui
  forms/mainwindow.ui  
  )

# and finally a resource file
set( RCS
  forms/icons.qrc
  )


#set(VTK_USE_HYBRID ON CACHE BOOL "doc" FORCE)
find_package(VTK REQUIRED)

if(VTK_FOUND)
  message(STATUS "Check VTK ..................................... yes (${VTK_VERSION})")
else(VTK_FOUND)
  message(FATAL_ERROR "Check VTK ..................................... no")
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
# cmake does not provide findqt5-modules
#message(STATUS "QT_Qmake_executable " ${QT_QMAKE_EXECUTABLE})
if($QT_QMAKE_EXECUTABLE STREQUAL "")
  FIND_PROGRAM(QT_QMAKE_EXECUTABLE NAMES qmake)
  if (NOT QT_QMAKE_EXECUTABL)
    message(WARNING "Check Qt          ...  no")
  endif()
endif()
# default qt version is 4
if("${DESIRED_QT_VERSION}" STREQUAL "")
  set(DESIRED_QT_VERSION "5")
endif()

# qt version > 5 or qt version < 3 are not supported
if( ${DESIRED_QT_VERSION} VERSION_GREATER 5 OR ${DESIRED_QT_VERSION} VERSION_LESS 3)
  message(WARNING "Desired QT version " ${DESIRED_QT_VERSION} " not supported. Taking 5")
  set(DESIRED_QT_VERSION "5")
endif()

# find packages according to qt version
if("${DESIRED_QT_VERSION}" STREQUAL "5")
  find_package(Qt5Widgets REQUIRED)
# - these find_packages are not necessary? -----------
  FIND_PACKAGE(Qt5Core REQUIRED)
  FIND_PACKAGE(Qt5Gui REQUIRED)
  FIND_PACKAGE(Qt5Network REQUIRED)
  FIND_PACKAGE(Qt5Xml REQUIRED)
  #-----------------------------------------------------
  set(QT5_INSTALLED TRUE)
  # # by default only QtCore and QtGui modules are enabled
  # # other modules must be enabled like this:
  set(QT_USE_QTXML TRUE)  # needed!
  set(QT_USE_QTNETWORK TRUE)
  MESSAGE(STATUS "Check Qt 5 .................................... yes (${Qt5Core_VERSION_STRING})")
elseif("${DESIRED_QT_VERSION}" STREQUAL "4")
   FIND_PACKAGE(Qt COMPONENTS QtXml QT_USE_QTNETWORK QtXmlPatterns REQUIRED)
   set(QT_USE_QTXML TRUE)  # needed!
   set(QT_USE_QTNETWORK TRUE)
endif()

# Disable automatic conversion from QString to ASCII 8-bit strings (char *)
# (Keeps code compatible with Qt/Mac/64bit)
if(APPLE)
  ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)
  # include the icns file in the target
  SET(SRCS ${SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns)
endif()

# this command will generate rules that will run rcc on all files from RCS
# in result RC_SRCS variable will contain paths to files produced by rcc
if(${QT4_INSTALLED})
  # and finally this will run moc:
  QT4_WRAP_CPP( MOC_HDRS ${MOC_HDRS} )
  # this will run uic on .ui files:
  QT4_WRAP_UI( UI_HDRS ${UIS} )
  QT4_ADD_RESOURCES( RCS ${RCS} )
  include( ${QT_USE_FILE} )
  ADD_EXECUTABLE( jpsvis MACOSX_BUNDLE WIN32
  ${SRCS}
  ${MOC_HDRS}
  ${UI_HDRS} 
  ${RCS}
  )
  TARGET_LINK_LIBRARIES( jpsvis ${QT_LIBRARIES})
elseif( ${QT5_INSTALLED})
  message(MESSAGE "libs: ${Qt5Widgets_LIBRARIES}  ${QT_LIBRARIES}")
  # http://www.kdab.com/using-cmake-with-qt-5/
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}" )
  qt5_wrap_ui( UI_HDRS ${UIS} )
  qt5_add_resources( RCS ${RCS} )

  ADD_EXECUTABLE( jpsvis MACOSX_BUNDLE WIN32
  ${SRCS}
  ${MOC_HDRS}
  ${UI_HDRS} 
  ${RCS}
  )
 qt5_use_modules(jpsvis Widgets)
 target_link_libraries(jpsvis ${Qt5Widgets_LIBRARIES} Qt5::Network Qt5::Xml)
else()
    MESSAGE(STATUS "Check Qt           ...  no")
endif()

# For Apple set the icns file containing icons

IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE JPSvis.icns) 
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(SRCS ${SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns)
ENDIF(APPLE)

# last thing we have to do is to tell CMake what libraries our executable needs,
# luckily FIND_PACKAGE prepared QT_LIBRARIES variable for us:
set(VTK_USE_GUISUPPORT  ON) 
set(VTK_USE_QT  ON)
# QVTK is for VTK 5.x. For VTK 6, use ${VTK_LIBRARIES} instead

if( ${QT_INSTALLED})
  SET(QTLIBS ${QT_LIBRARIES})
elseif( ${QT5_INSTALLED})
  SET(QTLIBS ${Qt5Widgets_LIBRARIES})
endif()

if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
   TARGET_LINK_LIBRARIES( jpsvis ${QTLIBS} QVTK) #qt5
  else()        
    #    TARGET_LINK_LIBRARIES( jpsvis  ${VTK_LIBRARIES} ${QT_LIBRARIES})
    TARGET_LINK_LIBRARIES( jpsvis  ${VTK_LIBRARIES} ${QTLIBS}) #qt5   
  endif()
else() # this else is not yet tested
  target_link_libraries(jpsvis vtkHybrid QVTK vtkViews ${QTLIBS})
endif() 
# ================================ Compiler flags
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
if(NOT MSVC)
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
message(STATUS "Checking for C++11 compiler ................... yes")
endif(NOT MSVC)
#---------------------
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Using compiler ................................ Clang (${CMAKE_CXX_COMPILER_VERSION})")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
if(CMAKE_COMPILER_IS_GNUCXX)
  message( STATUS "Using compiler ................................ g++ (${CMAKE_CXX_COMPILER_VERSION}")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
  message(STATUS "Debug flags: "  ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG} )
#else(CMAKE_BUILD_TYPE MATCHES Debug)
  # message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS} " ,  " ${CMAKE_CXX_FLAGS_RELEASE} )
endif(CMAKE_BUILD_TYPE MATCHES Debug)
#======================================================================================================================================

#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/jpsvis")
IF(APPLE)
  SET(plugin_dest_dir ${CMAKE_INSTALL_PREFIX}/jpsvis.app/Contents/MacOS)
  SET(qtconf_dest_dir ${CMAKE_INSTALL_PREFIX}/jpsvis.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/jpsvis.app")  
ENDIF(APPLE)

IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/jpsvis.exe")
ENDIF(WIN32)


#--------------------------------------------------------------------------------
# Install the QtTest application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS jpsvis
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
#SET(DIRS ${QT_LIBRARY_DIRS} ${QTPLUGINS})
if( ${QT4_INSTALLED})
  SET(DIRS ${QT_LIBRARIES})
elseif( ${QT5_INSTALLED})
  SET(DIRS ${QT_LIBRARY_DIRS})
endif()
message(STATUS "qt_lib_dirs  ${QT_LIBRARY_DIRS}")
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
  set_target_properties(jpsvis PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist.in)
  SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved.")
  SET(MACOSX_BUNDLE_BUNDLE_NAME "jpsvis")
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "${JPSVIS_VERSION}")
  SET(MACOSX_BUNDLE_LONG_VERSION_STRING "version ${JPSVIS_VERSION}")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${JPSVIS_VERSION}")
  SET(MACOSX_GUI_COPYRIGHT "Copyright (c) 2015 Forschungszentrum Juelich. All rights reserved.")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org")

ENDIF(APPLE)

SET(CPACK_RESOURCE_FILE_README "${README_FILE}")

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
set(CPACK_PACKAGE_NAME "jpsvis")
set(CPACK_PACKAGE_VENDOR "www.jupedsim.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "jpsvis - CPack Component Installation Example")
set(CPACK_PACKAGE_VERSION ${JPSVIS_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${JPSVIS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${JPSVIS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${JPSVIS_PATCH_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "JPSvis_dir")

if(APPLE)
  set(CPACK_GENERATOR "DragNDrop")
  set(CPACK_DMG_FORMAT "UDBZ")
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
  set(CPACK_SYSTEM_NAME "OSX")
  set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${JPSVIS_VERSION}")
endif()

include(CPack)

if(UNIX AND NOT APPLE)
  set(BIN_INSTALL_DIR "bin")
else()
  set(BIN_INSTALL_DIR ".")
  set(DOC_INSTALL_DIR ".")
endif()
message (STATUS "bin install dir: " ${BIN_INSTALL_DIR})
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL_DIR}")
include(InstallRequiredSystemLibraries)

if(APPLE)
  set(EXECUTABLE "${PROJECT_NAME}.app")
elseif(WIN32)
    set(EXECUTABLE "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
else()
    set(EXECUTABLE "${BIN_INSTALL_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
endif()

# https://github.com/artm/vision-ui-skeleton/blob/master/cmake/QArtmRelease.cmake
 # IF(APPLE)
#   SET(EXE_CONTENTS "${CMAKE_INSTALL_PREFIX}/${EXECUTABLE}/Contents")
#   FILE(WRITE
#     ${EXE_CONTENTS}/Resources/qt.conf
#     "[Paths]\nPlugins=PlugIns\n")
#   FILE(COPY ${QT_PLUGINS_DIR}/imageformats
#     DESTINATION ${EXE_CONTENTS}/PlugIns/
#     PATTERN "*_debug.*" EXCLUDE)
#   ADD_CUSTOM_COMMAND(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ruby ${PROJECT_SOURCE_DIR}/ruby/fixup/fixup.rb ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE}
#     COMMENT "Fixing up the app bundle")
# ENDIF(APPLE)


