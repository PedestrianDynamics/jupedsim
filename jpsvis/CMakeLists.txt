#cmake_minimum_required(VERSION 3.16)
#project(jpsvis LANGUAGES CXX C VERSION 0.8.3)

################################################################################
# Project options
################################################################################
#set(MORE_WARNINGS OFF CACHE BOOL
#    "Increase warning level to include not yet addressed issues"
#)
#set(WITH_BENCHMARKS OFF CACHE BOOL
#    "Also build micro benchmarks"
#)

################################################################################
# Project setup
################################################################################
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# set(CMAKE_COLOR_MAKEFILE ON)
# include(helper_functions)
# check_prefix_path()
# enforce_build_type_is_set()
#
# set(CMAKE_SKIP_BUILD_RPATH FALSE)
#
# # when building, don't use the install RPATH already
# # (but later on when installing)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# set(CMAKE_INSTALL_RPATH @executable_path/../Frameworks)
#
# # add the automatically determined parts of the RPATH
# # which point to directories outside the build tree to the install RPATH
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


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
# list(APPEND COMMON_COMPILE_OPTIONS
#     $<$<AND:$<BOOL:${MORE_WARNINGS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-Wall>
#     $<$<AND:$<BOOL:${MORE_WARNINGS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-Wextra>
#     $<$<AND:$<BOOL:${MORE_WARNINGS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-Wconversion>
#     $<$<AND:$<BOOL:${MORE_WARNINGS}>,$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>>:-Wsign-conversion>
#     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fdiagnostics-color=always>
#     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wimplicit-fallthrough>
#     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wpedantic>
#     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-gnu-zero-variadic-macro-arguments>
#     $<$<AND:$<BOOL:${MORE_WARNINGS}>,$<CXX_COMPILER_ID:MSVC>>:/W4>
#     $<$<AND:$<NOT:$<BOOL:${MORE_WARNINGS}>>,$<CXX_COMPILER_ID:MSVC>>:/W2>
#     $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
# )
#
# message( STATUS "System ........................................ ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_VERSION}, ${CMAKE_SYSTEM_PROCESSOR})" )
# message( STATUS "Generating  ................................... ${PROJECT_NAME} (${PROJECT_VERSION})")
#
# ################################################################################
# # Dependencies
# ################################################################################
# # VTK
# find_package(VTK 9.0
#     COMPONENTS
#         CommonCore
#         opengl
#         RenderingQt
#         RenderingCore
#         GUISupportQt
#         RenderingUI
#         InteractionStyle
#         RenderingAnnotation
#         IOImage
#     REQUIRED CONFIG
# )
# message(STATUS "Found VTK Version: ${VTK_VERSION}")
#
# # Qt5
# find_package(
#     Qt5 5.12
#     COMPONENTS Widgets Xml Core Gui
#     REQUIRED
# )
# message(STATUS "Found QT Version: ${Qt5_VERSION}")
# list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/forms")
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
#
# # GLM
# find_package(glm REQUIRED CONFIG)
# message(STATUS "Found GLM Version: ${GLM_VERSION}")
#
# # tinyxml - in tree 3rd party dependency
# add_library(tinyxml STATIC
#     src/tinyxml/tinystr.cpp
#     src/tinyxml/tinystr.h
#     src/tinyxml/tinyxml.cpp
#     src/tinyxml/tinyxml.h
#     src/tinyxml/tinyxmlerror.cpp
#     src/tinyxml/tinyxmlparser.cpp
# )
#
# target_include_directories(tinyxml PUBLIC
#     src/tinyxml
# )
#
# ################################################################################
# # Collect version information from git
# ################################################################################
# get_git_info()
# configure_file(src/BuildInfo.cpp.in ${CMAKE_BINARY_DIR}/src/BuildInfo.cpp @ONLY)

################################################################################
# libvis
################################################################################
add_library(vis STATIC
    ${CMAKE_BINARY_DIR}/generated/BuildInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/jpsvis.rc
    src/ApplicationState.h
    src/BuildInfo.h
    src/CLI.cpp
    src/CLI.h
    src/Frame.cpp
    src/Frame.h
    src/FrameElement.h
    src/IO/OutputHandler.cpp
    src/IO/OutputHandler.h
    src/InteractorStyle.cpp
    src/InteractorStyle.h
    src/Log.cpp
    src/Log.h
    src/MainWindow.cpp
    src/MainWindow.h
    src/Parsing.cpp
    src/Parsing.h
    src/RenderMode.h
    src/Settings.h
    src/TrailPlotter.cpp
    src/TrailPlotter.h
    src/TrajectoryData.cpp
    src/TrajectoryData.h
    src/Visualisation.cpp
    src/Visualisation.h
    src/general/Macros.h
    src/geometry/Building.cpp
    src/geometry/Building.h
    src/geometry/Crossing.cpp
    src/geometry/Crossing.h
    src/geometry/FacilityGeometry.cpp
    src/geometry/FacilityGeometry.h
    src/geometry/GeometryFactory.cpp
    src/geometry/GeometryFactory.h
    src/geometry/Goal.cpp
    src/geometry/Goal.h
    src/geometry/Hline.cpp
    src/geometry/Hline.h
    src/geometry/JPoint.cpp
    src/geometry/JPoint.h
    src/geometry/Line.cpp
    src/geometry/Line.h
    src/geometry/LinePlotter.cpp
    src/geometry/LinePlotter.h
    src/geometry/LinePlotter2D.cpp
    src/geometry/LinePlotter2D.h
    src/geometry/NavLine.cpp
    src/geometry/NavLine.h
    src/geometry/Obstacle.cpp
    src/geometry/Obstacle.h
    src/geometry/Point.cpp
    src/geometry/Point.h
    src/geometry/PointPlotter.cpp
    src/geometry/PointPlotter.h
    src/geometry/PointPlotter2D.cpp
    src/geometry/PointPlotter2D.h
    src/geometry/Room.cpp
    src/geometry/Room.h
    src/geometry/SubRoom.cpp
    src/geometry/SubRoom.h
    src/geometry/Transition.cpp
    src/geometry/Transition.h
    src/geometry/Wall.cpp
    src/geometry/Wall.h
    src/myqtreeview.cpp
    src/myqtreeview.h
    src/string_utils.cpp
    src/string_utils.h
    src/trains/train.h
)

target_include_directories(vis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

target_link_libraries(vis PUBLIC
    Qt5::Widgets
    Qt5::Xml
    Qt5::Core
    ${VTK_LIBRARIES}
    glm::glm
    tinyxml
)

vtk_module_autoinit(
    TARGETS vis
    MODULES ${VTK_LIBRARIES}
)

target_compile_options(vis PUBLIC
    ${COMMON_COMPILE_OPTIONS}
)

target_compile_definitions(vis
    PUBLIC
        # Qt will pull in wingdi.h which exposes among other things the ERROR macro.
        # This clashes with our Log::ERROR enum value. However we can savely disable
        # WINGDI macros because we are not using win32 gdi directly from our code.
        $<$<CXX_COMPILER_ID:MSVC>:NOGDI>
        $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
)

# See https://gitlab.kitware.com/cmake/cmake/-/issues/17456
set(TARGET_NAME "vis") # <- change this to your target name
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_autogen/include_$<CONFIG>)
else()
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_autogen/include)
endif()

target_include_directories(${TARGET_NAME} INTERFACE
  $<BUILD_INTERFACE:${AUTOGEN_INCLUDE_DIR}>
)

################################################################################
# jpsvis
################################################################################
set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns)
set_source_files_properties(${app_icon_macos}
    PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
)

add_executable(jpsvis
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${app_icon_macos}
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/jpsvis.rc
)

target_compile_options(jpsvis PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)

target_compile_definitions(jpsvis
    PRIVATE
        # Qt will pull in wingdi.h which exposes among other things the ERROR macro.
        # This clashes with our Log::ERROR enum value. However we can savely disable
        # WINGDI macros because we are not using win32 gdi directly from our code.
        $<$<CXX_COMPILER_ID:MSVC>:NOGDI>
        $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
)

target_link_libraries(jpsvis PUBLIC
    vis
    Qt5::Core
)

################################################################################
# Micro Benchmarks
################################################################################
# if(WITH_BENCHMARKS)
#     find_package(benchmark 1.6 REQUIRED CONFIG)
#     add_executable(benchmarks
#         benchmarks/parsing.cpp
#     )
#     target_link_libraries(benchmarks
#         vis
#         benchmark::benchmark
#         benchmark::benchmark_main
#     )
# endif()

################################################################################
# Packaging with CPack
################################################################################
# include(install_helper)
# create_package()
