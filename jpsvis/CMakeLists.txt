add_library(vis STATIC
    ${CMAKE_BINARY_DIR}/generated/BuildInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/icons.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/forms/jpsvis.rc
    src/ApplicationState.hpp
    src/BuildInfo.hpp
    src/CLI.cpp
    src/CLI.hpp
    src/Frame.cpp
    src/Frame.hpp
    src/FrameElement.hpp
    src/IO/OutputHandler.cpp
    src/IO/OutputHandler.hpp
    src/InteractorStyle.cpp
    src/InteractorStyle.hpp
    src/Log.cpp
    src/Log.hpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/Parsing.cpp
    src/Parsing.hpp
    src/RenderMode.hpp
    src/Settings.hpp
    src/TrailPlotter.cpp
    src/TrailPlotter.hpp
    src/TrajectoryData.cpp
    src/TrajectoryData.hpp
    src/Visualisation.cpp
    src/Visualisation.hpp
    src/general/Macros.hpp
    src/geometry/Building.cpp
    src/geometry/Building.hpp
    src/geometry/Crossing.cpp
    src/geometry/Crossing.hpp
    src/geometry/FacilityGeometry.cpp
    src/geometry/FacilityGeometry.hpp
    src/geometry/GeometryFactory.cpp
    src/geometry/GeometryFactory.hpp
    src/geometry/Goal.cpp
    src/geometry/Goal.hpp
    src/geometry/Hline.cpp
    src/geometry/Hline.hpp
    src/geometry/JPoint.cpp
    src/geometry/JPoint.hpp
    src/geometry/Line.cpp
    src/geometry/Line.hpp
    src/geometry/LinePlotter.cpp
    src/geometry/LinePlotter.hpp
    src/geometry/LinePlotter2D.cpp
    src/geometry/LinePlotter2D.hpp
    src/geometry/NavLine.cpp
    src/geometry/NavLine.hpp
    src/geometry/Obstacle.cpp
    src/geometry/Obstacle.hpp
    src/geometry/Point.cpp
    src/geometry/Point.hpp
    src/geometry/PointPlotter.cpp
    src/geometry/PointPlotter.hpp
    src/geometry/PointPlotter2D.cpp
    src/geometry/PointPlotter2D.hpp
    src/geometry/Room.cpp
    src/geometry/Room.hpp
    src/geometry/SubRoom.cpp
    src/geometry/SubRoom.hpp
    src/geometry/Transition.cpp
    src/geometry/Transition.hpp
    src/geometry/Wall.cpp
    src/geometry/Wall.hpp
    src/myqtreeview.cpp
    src/myqtreeview.hpp
    src/string_utils.cpp
    src/string_utils.hpp
    src/trains/train.hpp
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
