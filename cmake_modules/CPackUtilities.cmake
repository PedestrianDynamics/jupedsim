# create win-exe, dmg, and deb
#-------------------------------
# Structure:
# ├── LICENSE
# ├── README.md
# ├── JuPedSim.pdf
# ├── bin
# │   ├── jpscore
# │   └── jpsreport
# ├── jpscore_samples
# └── jpsreport_samples
#-------------------------------
function (cpack_write_deb_config)
  message(STATUS "Package generation - LINUX")
  list(APPEND CPACK_GENERATOR "DEB")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>=1.65), zlib1g-dev" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS, "jpsvis, jpseditor" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SECTION "science" PARENT_SCOPE)
  set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} PARENT_SCOPE)
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "David Haensel" PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "JuPedSim: framework for simulation and analysis of pedestrian dynamics" PARENT_SCOPE)
endfunction()

function (cpack_write_osx_config)
  message(STATUS "Package generation - MacOS")
  list(APPEND CPACK_GENERATOR "DragNDrop")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  Set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/jpscore/forms/background.png" PARENT_SCOPE)
   set(CPACK_DMG_DS_STORE_SETUP_SCRIPT
     "${CMAKE_SOURCE_DIR}/jpscore/forms/DS_Store.scpt" PARENT_SCOPE)
  set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON  PARENT_SCOPE)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.icns" PARENT_SCOPE)
  set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}" PARENT_SCOPE)
  set(CPACK_SYSTEM_NAME "OSX" PARENT_SCOPE)
endfunction()

function (cpack_write_windows_config)
  message(STATUS "Package generation - Windows")
  list(APPEND CPACK_GENERATOR "NSIS")
  set (CPACK_GENERATOR  ${CPACK_GENERATOR} PARENT_SCOPE)
  find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
  if(NSIS_PATH)
    message(STATUS "   + NSIS                                 YES ")
  else(NSIS_PATH)
    message(STATUS "   + NSIS                                 NO ")
  endif(NSIS_PATH)

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(VCPKG_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/")
  set(DIRS ${VCPKG_DIR})
  set(DLL_FILES ${VCPKG_DIR}/zlib1.dll ${VCPKG_DIR}/fmt.dll)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${DLL_FILES}")
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BUNDLE_RUNTIME_DESTINATION}")
  include(InstallRequiredSystemLibraries)

  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION bin
    COMPONENT applications)

  # some configs for installer
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.ico" PARENT_SCOPE)
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.ico" PARENT_SCOPE)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON PARENT_SCOPE)
  set(CPACK_NSIS_MODIFY_PATH ON PARENT_SCOPE)
  set(CPACK_NSIS_HELP_LINK "http://www.jupedsim.org/jupedsim_install_on_windows.html" PARENT_SCOPE)
  set(CPACK_NSIS_URL_INFO_ABOUT "http://www.jupedsim.org/" PARENT_SCOPE)
  set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PROJECT_NAME} PARENT_SCOPE)
  # ----------------------------
endfunction()

function (cpack_write_config)
  message(STATUS "Cpack write configs")
  set(CPACK_COMPONENTS_ALL applications PARENT_SCOPE)
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" PARENT_SCOPE)
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md" PARENT_SCOPE)
  set(CPACK_COMPONENTS_ALL applications jpscore_samples jpsreport_samples PARENT_SCOPE)
  set(CPACK_COMPONENT_CTDATA_GROUP "Sample files" PARENT_SCOPE)
  set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "jpscore and jpsreport" PARENT_SCOPE)
  set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "Sample files" PARENT_SCOPE)
  set(CPACK_COMPONENT_DATA_FILES_DESCRIPTION "Sample files to get started" PARENT_SCOPE)
  set(CPACK_PACKAGE_DESCRIPTION "jpscore and jpsreport" PARENT_SCOPE)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JuPedSim: framework for simulation and analysis of pedestrian dynamics" PARENT_SCOPE)
  set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH" PARENT_SCOPE)
  set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME} PARENT_SCOPE)
  set(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de" PARENT_SCOPE)
  set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}" PARENT_SCOPE)
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")

  install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION .
    COMPONENT Documents)

  install(FILES "${CMAKE_SOURCE_DIR}/README.md"
    DESTINATION .
    COMPONENT Documents)

  set(jpsguide "${CMAKE_SOURCE_DIR}/docs/jps_guide/JuPedSim.pdf")
  if(EXISTS "${jpsguide}")
    install(FILES "${jpsguide}"
      DESTINATION .
      COMPONENT Documents)
  endif()

  print_var(CPACK_SOURCE_PACKAGE_FILE_NAME)
  
  include(CPack)
endfunction()
