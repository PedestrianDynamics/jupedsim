################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(JuPedSim VERSION 0.10.0 LANGUAGES CXX)
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
set(BUILD_JPSCORE ON CACHE BOOL "Build jpscore cli application")
set(BUILD_JPSVIS ON CACHE BOOL "Build jpsvis gui application")

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
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:/W2>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/MP>
)

################################################################################
# Dependencies
################################################################################
find_package(fmt 8.0 REQUIRED CONFIG)
find_package(spdlog 1.9 REQUIRED CONFIG)
find_package(Boost 1.76 REQUIRED)
find_package(Threads REQUIRED)
find_package(glm REQUIRED CONFIG)
# Silence CGAL CMake warning. No example data for CGAL will be loaded anyways
set(CGAL_DATA_DIR "none")
set(WITH_CGAL_ImageIO OFF)
set(WITH_CGAL_Qt5 OFF)
if(UNIX)
    set(CGAL_DISABLE_GMP ON)
endif()
find_package(CGAL 5.4 REQUIRED)
find_library(POLY2TRI_LIB poly2tri)
if(POLY2TRI_LIB-NOTFOUND)
    message(FATAL_ERROR "Cannot find libpoly2tri.a")
else()
    message(STATUS "Found poly2tri: ${POLY2TRI_LIB}")
endif()
find_path(POLY2TRI_HEADER poly2tri.h
    PATH_SUFFIXES poly2tri
)
if(POLY2TRI_HEADER-NOTFOUND)
    message(FATAL_ERROR "Cannot find poly2tri header")
endif()
add_library(poly2tri UNKNOWN IMPORTED)
set_target_properties(poly2tri PROPERTIES
    IMPORTED_LOCATION ${POLY2TRI_LIB}
)
target_include_directories(poly2tri INTERFACE
    ${POLY2TRI_HEADER}/..
)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED CONFIG)

if(BUILD_JPSCORE)
    find_package(CLI11 2.1 REQUIRED CONFIG)
endif()
if(BUILD_JPSVIS)
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
endif()
if(BUILD_TESTS)
    find_package(PythonInterp 3 REQUIRED)
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
        DEPENDS jpscore
    )
    set(unittests_dependency_list libjupedsim-unittests libsimulator-unittests libshared-unittests)
    if(BUILD_JPSCORE)
        list(APPEND unittests_dependency_list libcore-unittests)
    endif()
    add_custom_target(unittests
        DEPENDS ${unittests_dependency_list}
    )

    if(BUILD_JPSCORE)
        add_custom_target(libcore-unittests
            COMMENT "Running libcore unittests"
            COMMAND $<TARGET_FILE:libcore-tests>
                    --gtest_output=xml:result-libcore-unittests.xml
            DEPENDS libcore-tests
        )
    endif()
    add_custom_target(libjupedsim-unittests
        COMMENT "Running libjupedsim unittests"
        COMMAND $<TARGET_FILE:libjupedsim-tests>
                --gtest_output=xml:result-libjupedsim-unittests.xml
        DEPENDS libjupedsim-tests
    )
    add_custom_target(libsimulator-unittests
        COMMENT "Running unit tests"
        COMMAND $<TARGET_FILE:libsimulator-tests>
                --gtest_output=xml:result-libsimulator-unittests.xml
        DEPENDS libsimulator-tests
    )
    add_custom_target(libshared-unittests
        COMMENT "Running unit tests"
        COMMAND $<TARGET_FILE:libshared-tests>
                --gtest_output=xml:result-libshared-unittests.xml
        DEPENDS libshared-tests
    )
endif()

if(UNIX)
    set(performancetests-wrapper-in ${CMAKE_SOURCE_DIR}/cmake_templates/run-performancetests.unix.in)
    set(performancetests-wrapper-out ${CMAKE_BINARY_DIR}/run-performancetests)
    configure_file(
        ${performancetests-wrapper-in}
        ${performancetests-wrapper-out}
        @ONLY
    )
endif()

################################################################################
# Add libraries / executables
################################################################################
if(BUILD_JPSCORE)
    add_subdirectory(jpscore)
    add_subdirectory(libcore)
endif()
if(BUILD_JPSVIS)
    add_subdirectory(jpsvis)
endif()
add_subdirectory(libjupedsim)
add_subdirectory(libshared)
add_subdirectory(libsimulator)
add_subdirectory(python_bindings_jupedsim)

################################################################################
# Code formating
################################################################################
if(UNIX)
    set(clang-format-version 14)
    find_program(CLANG_FORMAT
        NAMES
            clang-format-${clang-format-version}
            clang-format
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
            if(version MATCHES "^${clang-format-version}.*")
                message(STATUS "Found clang-format ${version}, add format-check and reformat targets")
                set(folders jpscore jpsvis libcore libshared libsimulator libjupedsim)
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
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(JPSCORE_INSTALLER_LOCATION ".")

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
    set(CPACK_NSIS_DISPLAY_NAME "JuPedSim")
    set(CPACK_NSIS_PACKAGE_NAME "JuPedSim")
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
    set(JPSCORE_INSTALLER_LOCATION "bin")
endif()
include(CPack)
