#########################################################
#                    INSTALL                            #
#########################################################
macro(create_package)
    write_package_information()
    write_package_content()
endmacro()

macro(write_package_information)
    set(CPACK_PACKAGE_FILE_NAME "jpsvis-installer-${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERISON_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERISON_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERISON_PATCH})
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_HOMEPAGE_URL "http://jupedsim.org")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_MONOLITHIC_INSTALL TRUE)
    set(CPACK_CREATE_DESKTOP_LINKS jpsvis)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    SET(CPACK_PACKAGE_DESCRIPTION "Visualisation module for JuPedSim")
    SET(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")

    if (APPLE)
        write_package_information_macos()
    elseif(WIN32)
        write_package_information_win()
    elseif(UNIX)
        write_package_information_unix()
    endif()
endmacro()

macro(write_package_content)
    include(InstallRequiredSystemLibraries)
    include(GNUInstallDirs)

    install(TARGETS jpsvis
        BUNDLE DESTINATION .
    )

    install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/samples
        DESTINATION .
    )

    if (APPLE)
        write_package_content_macos()
    elseif(WIN32)
        write_package_content_win()
    elseif(UNIX)
        write_package_content_unix()
    endif()

    include(CPack)
endmacro()

################################################################################
# MacOS Bundle
################################################################################
macro(write_package_information_macos)
    set(CPACK_GENERATOR "DragNDrop")

    SET(MACOSX_BUNDLE_ICON_FILE JPSvis.icns)
    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015-2021 Forschungszentrum Juelich. All rights reserved.")
    SET(MACOSX_BUNDLE_INFO_STRING "Visualisation module for  JuPedSim, a framework for simulation and analysis of pedestrian dynamics.")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "JPSvis")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "version ${PROJECT_VERSION}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
    SET(MACOSX_GUI_COPYRIGHT "Copyright (c) 2015-2021 Forschungszentrum Juelich. All rights reserved.")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org")
    set(CPACK_PACKAGE_EXECUTABLES "jpsvis;JPSVis")
endmacro()

macro(write_package_content_macos)
    set_target_properties(jpsvis PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )

    set_target_properties(jpsvis PROPERTIES
        INSTALL_RPATH @executable_path/../Frameworks
    )

    get_target_property(mocExe Qt5::moc IMPORTED_LOCATION)
    get_filename_component(qtBinDir "${mocExe}" DIRECTORY)

    find_program(DEPLOYQT_EXECUTABLE macdeployqt PATHS "${qtBinDir}" NO_DEFAULT_PATH)
    set(DEPLOY_OPTIONS [[${CMAKE_INSTALL_PREFIX}/jpsvis.app -verbose=2 ]])
    configure_file(${CMAKE_SOURCE_DIR}/cmake/deployapp.cmake.in deployapp.cmake @ONLY)
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

endmacro()

################################################################################
# Windows Installer
################################################################################
macro(write_package_information_win)
  set(CPACK_GENERATOR "NSIS" CACHE STRING "Generator used by CPack")
  set(CPACK_NSIS_HELP_LINK "www.jupedsim.org")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/jpsvis.ico")
  set(CPACK_NSIS_DISPLAY_NAME "JPSvis")
  set(CPACK_NSIS_PACKAGE_NAME "JPSvis")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/forms/jpsvis.ico")
  set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_NSIS_HELP_LINK})
  set(CPACK_NSIS_MENU_LINKS
    "bin/jpsvis.exe"
    "jpsvis")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
	  "CreateShortCut '$DESKTOP\\\\JPSvis.lnk' '$INSTDIR\\\\bin\\\\jpsvis.exe'"
  )
  set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
	  "Delete '$DESKTOP\\\\JPSvis.lnk'"
  )
endmacro()

macro(write_package_content_win)
    include(InstallRequiredSystemLibraries)
    include(GNUInstallDirs)
    install(TARGETS jpsvis)
    install(FILES
        $<TARGET_FILE_DIR:jpsvis>/brotlicommon.dll
        $<TARGET_FILE_DIR:jpsvis>/brotlidec.dll
        $<TARGET_FILE_DIR:jpsvis>/bz2.dll
        $<TARGET_FILE_DIR:jpsvis>/freetype.dll
        $<TARGET_FILE_DIR:jpsvis>/glew32.dll
        $<TARGET_FILE_DIR:jpsvis>/harfbuzz.dll
        $<TARGET_FILE_DIR:jpsvis>/icuuc69.dll
        $<TARGET_FILE_DIR:jpsvis>/jpeg62.dll
        $<TARGET_FILE_DIR:jpsvis>/libexpat.dll
        $<TARGET_FILE_DIR:jpsvis>/libpng16.dll
        $<TARGET_FILE_DIR:jpsvis>/lz4.dll
        $<TARGET_FILE_DIR:jpsvis>/lzma.dll
        $<TARGET_FILE_DIR:jpsvis>/pcre2-16.dll
        $<TARGET_FILE_DIR:jpsvis>/pugixml.dll
        $<TARGET_FILE_DIR:jpsvis>/Qt5Core.dll
        $<TARGET_FILE_DIR:jpsvis>/Qt5Gui.dll
        $<TARGET_FILE_DIR:jpsvis>/Qt5Widgets.dll
        $<TARGET_FILE_DIR:jpsvis>/Qt5Xml.dll
        $<TARGET_FILE_DIR:jpsvis>/tiff.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonColor-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonComputationalGeometry-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonDataModel-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonExecutionModel-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonMath-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonMisc-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonSystem-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkCommonTransforms-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkDICOMParser-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersExtraction-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersGeneral-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersGeometry-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersHybrid-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersModeling-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersSources-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersStatistics-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkFiltersTexture-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkGUISupportQt-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingColor-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingFourier-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingGeneral-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingHybrid-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingSources-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkImagingSources-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkInteractionStyle-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkInteractionWidgets-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkIOCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkIOImage-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkIOLegacy-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkIOXML-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkIOXMLParser-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkloguru-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkmetaio-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkParallelCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkParallelDIY-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingAnnotation-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingCore-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingFreeType-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingLabel-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingOpenGL2-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtkRenderingUI-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/vtksys-9.0.dll
        $<TARGET_FILE_DIR:jpsvis>/zlib1.dll
        $<TARGET_FILE_DIR:jpsvis>/zstd.dll
        $<TARGET_FILE_DIR:jpsvis>/icuin69.dll
        $<TARGET_FILE_DIR:jpsvis>/icudt69.dll
        TYPE BIN
    )
    install(DIRECTORY
        $<TARGET_FILE_DIR:jpsvis>/plugins
        TYPE BIN
    )
endmacro()

################################################################################
# Deb package
################################################################################
macro(write_package_information_unix)
    message(STATUS "Package generation - Deb")
    message(STATUS "not yet implemented")
endmacro()

macro(write_package_content_unix)
    message(STATUS "Package generation - Deb")
    message(STATUS "not yet implemented")
endmacro()
