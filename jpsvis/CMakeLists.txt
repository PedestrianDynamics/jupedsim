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

set_property(TARGET vis PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET vis PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

################################################################################
# jpsvis
################################################################################
set(app_icon_macos ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons/JPSvis.icns)
set_source_files_properties(${app_icon_macos}
    PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
)

add_executable(jpsvis
    WIN32
    MACOSX_BUNDLE
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

set_property(TARGET jpsvis PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET jpsvis PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
