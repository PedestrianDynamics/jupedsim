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
    set(CPACK_PACKAGE_EXECUTABLES "jpsvis")
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

###############################################################################
# WINDOWS
###############################################################################
function(cpack_write_windows_package_information)
    set(CPACK_GENERATOR "NSIS" CACHE STRING "Generator used by CPack")

    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/icons/jpsvis.ico")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "JPSvis")
    set(CPACK_NSIS_PACKAGE_NAME "JPSvis")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/forms/icons/jpsvis.ico")
    set(CPACK_NSIS_HELP_LINK "www.jupedsim.org")
    set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_NSIS_HELP_LINK})
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endfunction()


function(cpack_create_windows_package)
    include(InstallRequiredSystemLibraries)
    include(GNUInstallDirs)

    install(TARGETS petrack)

    # install Qwt and OpenCV
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/jpsvis${CMAKE_EXECUTABLE_SUFFIX}\"  \"\" \"\")
    ")

    get_target_property(mocExe Qt5::moc IMPORTED_LOCATION)
    get_filename_component(qtBinDir "${mocExe}" DIRECTORY)

    find_program(DEPLOYQT_EXECUTABLE windeployqt PATHS "${qtBinDir}" NO_DEFAULT_PATH)
endfunction()


###############################################################################
# UNIX
###############################################################################
function (write_package_information_unix)
    message(STATUS "Package generation - Unix - not yet implemented")
endfunction()

function (write_package_content_unix)
    message(STATUS "Cpack write configs - Unix - not yet implemented")
endfunction()
