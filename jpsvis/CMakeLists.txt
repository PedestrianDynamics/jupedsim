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

################################################################################
# Installation
################################################################################
install(TARGETS jpsvis
    BUNDLE
    DESTINATION .
)
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/examples/jpsvis
    DESTINATION .
)

if(APPLE)
    set_target_properties(jpsvis PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "JPSvis"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015-2022 Forschungszentrum Juelich. All rights reserved."
        MACOSX_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org"
        MACOSX_BUNDLE_ICON_FILE JPSvis.icns
        MACOSX_BUNDLE_INFO_STRING "Visualisation module for  JuPedSim, a framework for simulation and analysis of pedestrian dynamics."
        MACOSX_BUNDLE_LONG_VERSION_STRING "version ${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        INSTALL_RPATH @executable_path/../Frameworks
    )
    get_target_property(mocExe Qt5::moc IMPORTED_LOCATION)
    get_filename_component(qtBinDir "${mocExe}" DIRECTORY)
    find_program(DEPLOYQT_EXECUTABLE macdeployqt PATHS "${qtBinDir}" NO_DEFAULT_PATH)
    set(DEPLOY_OPTIONS "${CMAKE_BINARY_DIR}/bin/jpsvis.app")
    configure_file(${CMAKE_SOURCE_DIR}/cmake_templates/qt-deployapp-macos.cmake.in deployapp.cmake @ONLY)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/deployapp.cmake)
    install(CODE [[
      include(BundleUtilities)

      # This string is crazy-long enough that it's worth folding into a var...
      set (_plugin_file "$<TARGET_FILE_NAME:Qt5::QCocoaIntegrationPlugin>")

      # Ditto the output paths for our installation
      set (_appdir "${CMAKE_INSTALL_PREFIX}/jpsvis.app")
      set (_outdir "${_appdir}/Contents/PlugIns/platforms")

      file(INSTALL DESTINATION "${_outdir}"
        TYPE FILE FILES "$<TARGET_FILE:Qt5::QCocoaIntegrationPlugin>")

      fixup_bundle("${_appdir}/Contents/MacOS/jpsvis" "${_outdir}/${_plugin_file}" "")
    ]] COMPONENT Runtime)

elseif(WIN32)
    set(origin "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    install(FILES
        ${origin}/bin/Qt5Core.dll
        ${origin}/bin/Qt5Gui.dll
        ${origin}/bin/Qt5Widgets.dll
        ${origin}/bin/Qt5Xml.dll
        ${origin}/bin/brotlicommon.dll
        ${origin}/bin/brotlidec.dll
        ${origin}/bin/bz2.dll
        ${origin}/bin/fmt.dll
        ${origin}/bin/freetype.dll
        ${origin}/bin/glew32.dll
        ${origin}/bin/harfbuzz.dll
        ${origin}/bin/icudt69.dll
        ${origin}/bin/icuin69.dll
        ${origin}/bin/icuuc69.dll
        ${origin}/bin/jpeg62.dll
        ${origin}/bin/libexpat.dll
        ${origin}/bin/libpng16.dll
        ${origin}/bin/lz4.dll
        ${origin}/bin/lzma.dll
        ${origin}/bin/pcre2-16.dll
        ${origin}/bin/pugixml.dll
        ${origin}/bin/spdlog.dll
        ${origin}/bin/tiff.dll
        ${origin}/bin/vtkCommonColor-9.0.dll
        ${origin}/bin/vtkCommonComputationalGeometry-9.0.dll
        ${origin}/bin/vtkCommonCore-9.0.dll
        ${origin}/bin/vtkCommonDataModel-9.0.dll
        ${origin}/bin/vtkCommonExecutionModel-9.0.dll
        ${origin}/bin/vtkCommonMath-9.0.dll
        ${origin}/bin/vtkCommonMisc-9.0.dll
        ${origin}/bin/vtkCommonSystem-9.0.dll
        ${origin}/bin/vtkCommonTransforms-9.0.dll
        ${origin}/bin/vtkDICOMParser-9.0.dll
        ${origin}/bin/vtkFiltersCore-9.0.dll
        ${origin}/bin/vtkFiltersExtraction-9.0.dll
        ${origin}/bin/vtkFiltersGeneral-9.0.dll
        ${origin}/bin/vtkFiltersGeometry-9.0.dll
        ${origin}/bin/vtkFiltersHybrid-9.0.dll
        ${origin}/bin/vtkFiltersModeling-9.0.dll
        ${origin}/bin/vtkFiltersSources-9.0.dll
        ${origin}/bin/vtkFiltersStatistics-9.0.dll
        ${origin}/bin/vtkFiltersTexture-9.0.dll
        ${origin}/bin/vtkGUISupportQt-9.0.dll
        ${origin}/bin/vtkIOCore-9.0.dll
        ${origin}/bin/vtkIOImage-9.0.dll
        ${origin}/bin/vtkIOLegacy-9.0.dll
        ${origin}/bin/vtkIOXML-9.0.dll
        ${origin}/bin/vtkIOXMLParser-9.0.dll
        ${origin}/bin/vtkImagingColor-9.0.dll
        ${origin}/bin/vtkImagingCore-9.0.dll
        ${origin}/bin/vtkImagingFourier-9.0.dll
        ${origin}/bin/vtkImagingGeneral-9.0.dll
        ${origin}/bin/vtkImagingHybrid-9.0.dll
        ${origin}/bin/vtkImagingSources-9.0.dll
        ${origin}/bin/vtkImagingSources-9.0.dll
        ${origin}/bin/vtkInteractionStyle-9.0.dll
        ${origin}/bin/vtkInteractionWidgets-9.0.dll
        ${origin}/bin/vtkParallelCore-9.0.dll
        ${origin}/bin/vtkParallelDIY-9.0.dll
        ${origin}/bin/vtkRenderingAnnotation-9.0.dll
        ${origin}/bin/vtkRenderingCore-9.0.dll
        ${origin}/bin/vtkRenderingFreeType-9.0.dll
        ${origin}/bin/vtkRenderingLabel-9.0.dll
        ${origin}/bin/vtkRenderingOpenGL2-9.0.dll
        ${origin}/bin/vtkRenderingUI-9.0.dll
        ${origin}/bin/vtkloguru-9.0.dll
        ${origin}/bin/vtkmetaio-9.0.dll
        ${origin}/bin/vtksys-9.0.dll
        ${origin}/bin/zlib1.dll
        ${origin}/bin/zstd.dll
        TYPE BIN
    )
    install(DIRECTORY
        ${origin}/plugins/platforms
        TYPE BIN
    )
endif()
