# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER:STRING=clang++   -DCMAKE_C_COMPILER:STRING=clang .
# -DCMAKE_BUILD_TYPE:STRING=Debug (default Release)
# -DBUILD_TESTING=ON (default OFF)
#--------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(jpsvis LANGUAGES CXX C VERSION 0.8.3)

################################################################################
# Project setup
################################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_COLOR_MAKEFILE ON)

include(helper_functions)
check_prefix_path()
enforce_build_type_is_set()

################################################################################
# Compilation flags
################################################################################
# Note: Setting global compile flags via CMAKE_CXX_FLAGS has the drawback that
#       generator expressions cannot be used. This leads to all kind of
#       conditional adding of flags. It is generally preferable to use generator
#       expresssions.
#
# WARNING: Do not break the lines, each option has to be on its own line or
#          CMake will enclose multiple flags in '' which the compiler then
#          treats as a single flag and does not understand.
list(APPEND COMMON_COMPILE_OPTIONS
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wextra>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wconversion>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wimplicit-fallthrough>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wsign-conversion>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fdiagnostics-color=always>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wpedantic>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-gnu-zero-variadic-macro-arguments>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
)

message( STATUS "System ........................................ ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_VERSION}, ${CMAKE_SYSTEM_PROCESSOR})" )
message( STATUS "Generating  ................................... ${PROJECT_NAME} (${PROJECT_VERSION})")

################################################################################
# Dependencies
################################################################################
# VTK
find_package(VTK 8.0 REQUIRED CONFIG)
message(STATUS "VTK_VERSION: ${VTK_VERSION}")

if (VTK_VERSION VERSION_LESS "8.90.0")
    include(${VTK_USE_FILE})
endif()

# Qt
find_package(
    Qt5 5.12
    COMPONENTS Widgets Xml Core Gui Network
    REQUIRED
)

#**********************************************************
# Qt and Misc Stuff                                       *
#**********************************************************
set(CMAKE_AUTOUIC ON)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/forms" "${CMAKE_SOURCE_DIR}/src")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(AUTOGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_autogen")

#**********************************************************
# Create library and executable                           *
#**********************************************************
set(app_icon_macos ${CMAKE_SOURCE_DIR}/forms/icons/JPSvis.icns)
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(jpsvis MACOSX_BUNDLE WIN32 src/main.cpp ${app_icon_macos})

target_compile_options(jpsvis PRIVATE ${COMMON_COMPILE_OPTIONS})

#**********************************************************
# Linking Qt and VTK
#**********************************************************
get_git_info()
target_link_libraries(jpsvis PUBLIC
        Qt5::Widgets
        Qt5::Xml
        Qt5::Core
        Qt5::Network
        ${VTK_LIBRARIES}
        git-info
)

# vtk_module_autoinit is needed
if (VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
    vtk_module_autoinit(
            TARGETS jpsvis
            MODULES ${VTK_LIBRARIES}
    )
endif()

#**************************************************************
# SOURCES                                                     *
#**************************************************************
target_include_directories(jpsvis PUBLIC "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/forms" ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set(SRCS
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
    src/Log.cpp
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
    src/myqtreeview.cpp
    forms/jpsvis.rc
)

set(HDR
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
    src/train.h
    src/Log.h
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
    src/myqtreeview.h
)

IF(APPLE)
    set(MAC_FIX_SRC src/fix/osx_thread_fix.mm) #.mm
    set(MAC_FIX_HDR src/fix/osx_thread_fix.h) #.h

    set(SRCS ${SRCS} ${MAC_FIX_SRC})
    set(HDR ${HDR} ${MAC_FIX_HDR})
ENDIF(APPLE)

set(UIS
    forms/settings.ui
    forms/mainwindow.ui
)

set(RCS
    forms/icons.qrc
)

target_sources(jpsvis PRIVATE ${SRCS} ${HDR} ${UIS} ${HDR} ${RCS})

target_compile_definitions(jpsvis PUBLIC
    JPSVIS_VERSION="${PROJECT_VERSION}"
)

################################################################################
# Packaging with CPack
################################################################################
#include(install_helper)
#if(WIN32 AND NOT UNIX)
#    cpack_write_windows_config()
#endif()
#if(APPLE)
#    cpack_write_osx_config()
#endif()
#
#cpack_write_config()
