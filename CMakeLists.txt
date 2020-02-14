# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER=clang++
# -DCMAKE_BUILD_TYPE=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF) for python tests
# -DBUILD_CPPUNIT_TEST=ON (default OFF) for unit tests
# -D Boost_NO_SYSTEM_PATHS=true (default false) -D BOOST_ROOT=PATH_where_to_find_boost
#--------------------------------------------------------------------------
################################################################################
# Project setup
################################################################################
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(JPScore VERSION 0.8.4 LANGUAGES CXX)
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

check_prefix_path()

################################################################################
# Optional features
################################################################################
set(BUILD_TESTING OFF CACHE BOOL "Build with integration tests")
print_var(BUILD_TESTING)

set(BUILD_CPPUNIT_TEST OFF CACHE BOOL "Build with unit tests")
print_var(BUILD_CPPUNIT_TEST)

set(BUILD_DOC OFF CACHE BOOL "Build doxygen documentation")
print_var(BUILD_DOC)

# Due to old OpenMP implementation in MSVC we disable OpenMP on Windows for the moment.
if(${CMAKE_SYSTEM} MATCHES "Windows")
    set(USE_OPENMP Off)
    message(WARNING "OpenMP is disabled on Windows")
else()
    set(USE_OPENMP ON CACHE BOOL "Build with OpenMP")
endif()
print_var(USE_OPENMP)

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
)

################################################################################
# Dependencies
################################################################################
add_subdirectory(third-party)

# libfmt
find_package(fmt 6.0 REQUIRED CONFIG)

# spdlog
find_package(spdlog 1.3 REQUIRED CONFIG)

# CLI11
find_package(CLI11 1.8 REQUIRED CONFIG)

# Boost
find_package(Boost 1.65 REQUIRED ${BOOST_COMPONENTS})

# std::filesystem needs to be linked on gcc < 9
add_library(fs INTERFACE)
target_link_libraries(fs INTERFACE
    $<$<PLATFORM_ID:Linux>:stdc++fs>
)

# CMakes find module for openmp (FindOpenMP) is currently (cmake 1.15.1) broken
# in several ways:
# 1st. Version detection does only work on first cmake generation. E.g. if
# version 3.1 is found but verison 4 required cmake generation will fail
# rightfully on the first call but subsequent calls will pass, i.e. silently use
# 3.1 even though 4 was strictly required.
# See Issue: https://gitlab.kitware.com/cmake/cmake/issues/19150
#
# 2nd. FindOpenMP is broken on MacOS Mojave, it cannot find openmp at all
# currently. See Issue:https://gitlab.kitware.com/cmake/cmake/issues/18520
#
# Due to the issues mentioned above we have to do some extra work to make sure
# openmp is found. Note this does not do ANY version checking!
if(USE_OPENMP)
    if(${CMAKE_SYSTEM} MATCHES "Darwin" AND ${CMAKE_CXX_COMPILER_ID} MATCHES "AppleClang")
        find_library(OpenMP_LIBRARY
            NAMES omp
            PATHS
                /opt/local
                /opt/local/lib/libomp
        )
        find_path(OpenMP_INCLUDE_DIR
            omp.h
            PATHS
                /opt/local
                /opt/local/include/libomp
        )

        mark_as_advanced(OpenMP_LIBRARY OpenMP_INCLUDE_DIR)

        include(FindPackageHandleStandardArgs)
        find_package_handle_standard_args(OpenMP DEFAULT_MSG
            OpenMP_LIBRARY OpenMP_INCLUDE_DIR)

        if (OpenMP_FOUND)
            set(OpenMP_LIBRARIES ${OpenMP_LIBRARY})
            set(OpenMP_INCLUDE_DIRS ${OpenMP_INCLUDE_DIR})
            set(OpenMP_COMPILE_OPTIONS -Xpreprocessor -fopenmp)

            add_library(OpenMP::OpenMP_CXX SHARED IMPORTED)
            set_target_properties(OpenMP::OpenMP_CXX PROPERTIES
                IMPORTED_LOCATION ${OpenMP_LIBRARIES}
                INTERFACE_INCLUDE_DIRECTORIES "${OpenMP_INCLUDE_DIRS}"
                INTERFACE_COMPILE_OPTIONS "${OpenMP_COMPILE_OPTIONS}"
            )
        endif()
    else()
        find_package(OpenMP REQUIRED)
    endif()
else()
    list(APPEND COMMON_COMPILE_OPTIONS
        # Disable warnigns about omp pragmas
         $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wno-unknown-pragmas>
    )
endif()

find_package(Threads REQUIRED)
find_package(PythonInterp 3 REQUIRED)

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
if(BUILD_CPPUNIT_TEST OR BUILD_TESTING)
    enable_testing()
endif()

################################################################################
# Add libraries / executables
################################################################################
add_subdirectory(libcore)
add_subdirectory(jpscore)
add_subdirectory(jpsreport)

################################################################################
# Integration tests
################################################################################
if (BUILD_TESTING)
  add_subdirectory(systemtest)
endif (BUILD_TESTING)

################################################################################
# Packaging with CPack
################################################################################
include(CPackUtilities)
IF(UNIX AND NOT APPLE)
  cpack_write_deb_config()
endif()
if(WIN32 AND NOT UNIX)
  cpack_write_windows_config()
endif()
if(APPLE)
  cpack_write_osx_config()
endif()

cpack_write_config()

