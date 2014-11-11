# ------------------------- Begin Generic CMake Variable Logging ------------------
# todo: how to check these:
# sudo apt-get install libvtk-java
# sudo apt-get install python-vtk

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
 # set project's name
project( JPSvis )

set(CMAKE_COLOR_MAKEFILE ON)
set(JPSVIS_MAJOR_VERSION 0)
set(JPSVIS_MINOR_VERSION 5)
set(JPSVIS_PATCH_VERSION 0)
set(JPSVIS_VERSION
  ${JPSVIS_MAJOR_VERSION}.${JPSVIS_MINOR_VERSION}.${JPSVIS_PATCH_VERSION})
message( STATUS "JPSVIS_VERSION: " ${JPSVIS_VERSION} )

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
  
  # *.ui files
  set( SAMPLE_UIS
    forms/settings.ui
    forms/mainwindow.ui  
  )
  
# and finally a resource file
set( SAMPLE_RCS
  forms/icons.qrc
  )
  
# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
# set( QT_USE_QT3SUPPORT TRUE )   
set( QT_USE_QTXML TRUE ) # needed!

# set(QT_USE_QTOPENGL TRUE)
# set(QT_USE_QTNETWORK)
# set(QT_USE_QTOPENGL)
# set(QT_USE_QTSQL)
# set(QT_USE_QTSVG)
# set(QT_USE_QTTEST)
# set(QT_USE_QTDBUS)
# set(QT_USE_QTSCRIPT)
# set(QT_USE_QTWEBKIT)
# set(QT_USE_QTXMLPATTERNS)
# set(QT_USE_PHONON)


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
  message(WARNING: JPSvis is not tested with vtk ${VTK_VERSION})
endif()
#   # Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
#find_package(Qt5Widgets REQUIRED QUIET)

find_package(Qt4 REQUIRED)
#find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Test REQUIRED)

#include(${QT_USE_FILE})
  # this command finds Qt4 libraries and sets all required variables
  # note that it's Qt4, not QT4 or qt4
FIND_PACKAGE( Qt4 REQUIRED )
  
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
  
# here we instruct CMake to build JPSvis executable from all of the source files
  ADD_EXECUTABLE( JPSvis ${SAMPLE_SRCS} 
    ${SAMPLE_MOC_HDRS}
    ${SAMPLE_UI_HDRS} 
    ${SAMPLE_RESOURCES_RCS}
    )
  
  # last thing we have to do is to tell CMake what libraries our executable needs,
  # luckily FIND_PACKAGE prepared QT_LIBRARIES variable for us:
set(VTK_USE_GUISUPPORT  ON) 
set(VTK_USE_QT  ON )
set(VTK_USE_QVTK_QTOPENGL ON )

#TARGET_LINK_LIBRARIES( JPSvis ${QT_LIBRARIES} QVTK) # QVTK is for VTK 5.x. For VTK 6, use ${VTK_LIBRARIES} instead

if(${VTK_VERSION} VERSION_LESS "6")
  TARGET_LINK_LIBRARIES( JPSvis ${QT_LIBRARIES} QVTK)
else()
  TARGET_LINK_LIBRARIES( JPSvis  ${VTK_LIBRARIES} QVTK)
endif()
