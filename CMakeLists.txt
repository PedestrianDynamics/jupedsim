################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(JuPedSim VERSION 0.9.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(helper_functions)

print_var(PROJECT_VERSION)

# Set default build type to release
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(USE_IPO ON)

check_prefix_path()

################################################################################
# Optional features
################################################################################
set(BUILD_TESTS OFF CACHE BOOL "Build tests")
print_var(BUILD_TESTS)

set(BUILD_DOC OFF CACHE BOOL "Build doxygen documentation")
print_var(BUILD_DOC)

set(BUILD_WITH_ASAN OFF CACHE BOOL
  "Build with address sanitizer support. Linux / macOS only.")
print_var(BUILD_WITH_ASAN)
if(BUILD_WITH_ASAN AND ${CMAKE_SYSTEM} MATCHES "Windows")
    message(FATAL_ERROR "Address sanitizer builds are not supported on Windows")
endif()
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
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fdiagnostics-color=always>
    $<$<CXX_COMPILER_ID:MSVC>:/W2>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

################################################################################
# Dependencies
################################################################################
find_package(fmt 8.0 REQUIRED CONFIG)
find_package(spdlog 1.9 REQUIRED CONFIG)
find_package(CLI11 2.1 REQUIRED CONFIG)
find_package(Boost 1.76 REQUIRED)
# std::filesystem needs to be linked on gcc < 9
add_library(fs INTERFACE)
target_link_libraries(fs INTERFACE
    $<$<PLATFORM_ID:Linux>:stdc++fs>
)
find_package(Threads REQUIRED)
find_package(PythonInterp 3 REQUIRED)
find_package(VTK 9.0
    COMPONENTS
        CommonCore
        opengl
        RenderingQt
        RenderingCore
        GUISupportQt
        RenderingUI
        InteractionStyle
        RenderingAnnotation
        IOImage
    REQUIRED CONFIG
)
find_package(
    Qt5 5.12
    COMPONENTS Widgets Xml Core Gui
    REQUIRED
)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/forms")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(glm REQUIRED CONFIG)
if(BUILD_TESTS)
    find_package(Catch2 REQUIRED)
    find_package(GTest 1.11 REQUIRED CONFIG)
endif()
# remaining in-tree dependencies
add_subdirectory(third-party)
################################################################################
# VCS info
################################################################################
find_package(Git QUIET)
find_program(GIT_SCM git DOC "Git version control")
mark_as_advanced(GIT_SCM)
find_file(GITDIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
if (GIT_SCM AND GITDIR)
    # the commit's SHA1, and whether the building workspace was dirty or not
    # describe --match=NeVeRmAtCh --always --tags --abbrev=40 --dirty
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" --no-pager describe --tags --always --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    # branch
    execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # the date of the commit
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND
    "${GIT_EXECUTABLE}" describe --tags --abbrev=0
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the subject of the commit
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    # remove # from subject
    string(REGEX REPLACE "[\#\"]+"
       "" GIT_COMMIT_SUBJECT
       ${GIT_COMMIT_SUBJECT})
else()
    message(STATUS "Not in a git repo")
    set(GIT_SHA1 "UNKNOWN")
    set(GIT_DATE "UNKNOWN")
    set(GIT_COMMIT_SUBJECT "UNKNOWN")
    set(GIT_BRANCH "UNKNOWN")
    set(GIT_TAG "UNKNOWN")
endif()

add_library(git-info INTERFACE)
target_compile_definitions(git-info INTERFACE
    GIT_COMMIT_HASH="${GIT_SHA1}"
    GIT_COMMIT_DATE="${GIT_DATE}"
    GIT_TAG="${GIT_TAG}"
    GIT_COMMIT_SUBJECT="${GIT_COMMIT_SUBJECT}"
    GIT_BRANCH="${GIT_BRANCH}"
)
configure_file(cmake_templates/BuildInfo.cpp.in ${CMAKE_BINARY_DIR}/generated/BuildInfo.cpp @ONLY)
################################################################################
# Documentation
################################################################################
if(BUILD_DOC)
    find_package(Doxygen REQUIRED)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    # generate file titlepage.tex based on titlepage.tex.in
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/jps_guide/titlepage.tex.in
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/jps_guide/titlepage.tex
        @ONLY
    )
    add_custom_target(guide
        "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/docs/jps_guide/make_guide.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/jps_guide
        COMMENT "Generating guide"
        VERBATIM
    )
endif(BUILD_DOC)

################################################################################
# Testing
################################################################################
if(BUILD_TESTS)
    enable_testing()
    if(UNIX)
        set(pytest-wrapper-in ${CMAKE_SOURCE_DIR}/cmake_templates/run-systemtests.unix.in)
        set(pytest-wrapper-out ${CMAKE_BINARY_DIR}/run-systemtests)
    else()
        set(pytest-wrapper-in ${CMAKE_SOURCE_DIR}/cmake_templates/run-systemtests.windows.in)
        set(pytest-wrapper-out ${CMAKE_BINARY_DIR}/run-systemtests.cmd)
    endif()
    configure_file(
        ${pytest-wrapper-in}
        ${pytest-wrapper-out}
        @ONLY
    )
    add_custom_target(tests
        DEPENDS systemtests unittests
    )
    add_custom_target(systemtests
        COMMENT "Running system tests"
        COMMAND ${pytest-wrapper-out} -vv -s
                --basetemp=${CMAKE_BINARY_DIR}/systemtest-out
                --junit-xml=result-systemtests.xml
        COMMAND ctest -T test
        DEPENDS jpscore
    )
    add_custom_target(unittests
        COMMENT "Running unit tests"
        COMMAND echo "GTest"
        COMMAND $<TARGET_FILE:libcore-tests>
                --gtest_output=xml:result-unittests.xml
        COMMAND echo "Catch2"
        COMMAND $<TARGET_FILE:catch-unittests>
                --reporter junit
                --out=result-unittests2.xml
        DEPENDS libcore-tests catch-unittests
    )
endif()

################################################################################
# Add libraries / executables
################################################################################
add_subdirectory(jpscore)
add_subdirectory(jpsvis)
add_subdirectory(libcore)
add_subdirectory(libshared)

################################################################################
# Code formating
################################################################################
if(UNIX)
    find_program(CLANG_FORMAT
        NAMES
            clang-format
            clang-format-13
    )
    if(CLANG_FORMAT)
        execute_process(
            COMMAND ${CLANG_FORMAT} --version
            OUTPUT_VARIABLE version_string
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(version_string MATCHES "^.*clang-format version .*")
            string(REGEX REPLACE
                "^.*clang-format version ([.0-9]+).*"
                "\\1"
                version
                "${version_string}"
            )
            if(version MATCHES "^13.*")
                message(STATUS "Found clang-format ${version}, add format-check and reformat targets")
                set(folders jpscore jpsreport libcore)
                add_custom_target(check-format
                    COMMENT "Checking format with clang-format"
                    COMMAND find ${folders}
                        -name '*.cpp'
                        -o -name '*.c'
                        -o -name '*.h'
                        -o -name '*.hpp' | xargs ${CLANG_FORMAT}
                        -n -Werror --style=file
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
                add_custom_target(reformat
                    COMMENT "Reformatting code with clang-format"
                    COMMAND find ${folders}
                        -name '*.cpp'
                        -o -name '*.c'
                        -o -name '*.h'
                        -o -name '*.hpp' | xargs ${CLANG_FORMAT}
                        -i --style=file
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
            endif()
        endif()
    endif()
endif()

################################################################################
# Integration tests
################################################################################
if (BUILD_TESTS)
  # Find libraries needed by python tests
  find_python_library(numpy)
  find_python_library(scipy)
  find_python_library(pandas)
  find_python_library(matplotlib)
  add_subdirectory(systemtest)
endif (BUILD_TESTS)

################################################################################
# Packaging
################################################################################
include(InstallRequiredSystemLibraries)
include(GNUInstallDirs)

set(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
set(CPACK_PACKAGE_DESCRIPTION "JuPedSim")
set(CPACK_PACKAGE_HOMEPAGE_URL "http://jupedsim.org")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_PACKAGE_EXECUTABLES "jpscore;JuPedSim Simulator;jpsvis;JuPedSim Visualization")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/jpscore/forms/background.png")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/jpscore/forms/jpscore.ico")
    # MacOS PList settings
    set(MACOSX_BUNDLE_BUNDLE_NAME "JuPedSim")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015-2022 Forschungszentrum Juelich. All rights reserved.")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "www.jupedsim.org")
    set(MACOSX_BUNDLE_ICON_FILE JPSvis.icns)
    set(MACOSX_BUNDLE_INFO_STRING "JuPedSim: A framework for simulation and analysis of pedestrian dynamics.")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "version ${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_HELP_LINK "www.jupedsim.org")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/jpsvis/forms/jpsvis.ico")
    set(CPACK_NSIS_DISPLAY_NAME "JPSvis")
    set(CPACK_NSIS_PACKAGE_NAME "JPSvis")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/jpsvis/forms/jpsvis.ico")
    set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_NSIS_HELP_LINK})
    set(CPACK_NSIS_MENU_LINKS
        "bin/jpsvis.exe"
        "jpsvis"
    )
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
      "CreateShortCut '$DESKTOP\\\\JPSvis.lnk' '$INSTDIR\\\\bin\\\\jpsvis.exe'"
    )
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
      "Delete '$DESKTOP\\\\JPSvis.lnk'"
    )
endif()

install(TARGETS jpscore
    DESTINATION .
)

install(TARGETS jpsvis
    BUNDLE
    DESTINATION .
)

install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/examples
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
include(CPack)
