function (cpack_write_deb_config)
  
  message(STATUS "Package generation - LINUX")
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org")
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS, "jpsvis, jpsreport, jpseditor")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  set(CPACK_DEBIAN_PACKAGE_SECTION "science")
  set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endfunction()

function (cpack_write_osx_config)
    message(STATUS "Package generation - MacOS")
    

    install(TARGETS jpscore
      BUNDLE DESTINATION . COMPONENT Runtime
      RUNTIME DESTINATION bin COMPONENT Runtime
    )

endfunction()

function (cpack_write_windows_config)
    message(STATUS "Package generation - Windows")
    set(CPACK_GENERATOR "NSIS")
    find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
    if(NSIS_PATH)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
        message(STATUS "   + NSIS                                 YES ")
    else(NSIS_PATH)
        message(STATUS "   + NSIS                                 NO ")
    endif(NSIS_PATH)

    
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
    message(STATUS "System Dependencies: " ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.ico")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_HELP_LINK "http://www.jupedsim.org/jupedsim_install_on_windows.html")
    set(CPACK_NSIS_URL_INFO_ABOUT "http://www.jupedsim.org/")
    set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PROJECT_NAME})

    set(DIRS ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/build/deps) #todo hardcoded

    install(CODE "include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")")

    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_BACKGROUND_IMAGE  "${CMAKE_SOURCE_DIR}/jpscore/forms/JPScore.ico")

    
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      DESTINATION sys
      COMPONENT applications)  
endfunction()


function (cpack_write_config)
  
  set(CPACK_COMPONENTS_ALL applications)
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
  set(CPACK_COMPONENTS_ALL applications demos)
  set(CPACK_COMPONENT_CTDATA_GROUP "Demos")
  set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "jpscore and dlls")
  set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "demo files")
  set(CPACK_COMPONENT_DATA_FILES_DESCRIPTION "demo files to get started")
  set(CPACK_PACKAGE_DESCRIPTION "Simulation core of JuPedSim")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The simulation core of JuPedSim, a framework for simulation and analysis of pedestrian dynamics")
  set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
  set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
  set(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
  print_var(CPACK_SOURCE_PACKAGE_FILE_NAME)
  include(CPack)
endfunction()
