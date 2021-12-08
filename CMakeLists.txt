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
include(code-coverage)

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
set(BUILD_TESTS OFF CACHE BOOL "Build tests")
print_var(BUILD_TESTS)

set(BUILD_DOC OFF CACHE BOOL "Build doxygen documentation")
print_var(BUILD_DOC)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT USE_IPO)
else()
    set(USE_IPO ON)
endif()

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
find_package(Boost 1.74 REQUIRED)
# std::filesystem needs to be linked on gcc < 9
add_library(fs INTERFACE)
target_link_libraries(fs INTERFACE
    $<$<PLATFORM_ID:Linux>:stdc++fs>
)

find_package(Threads REQUIRED)
find_package(PythonInterp 3 REQUIRED)
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
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake_templates/run-systemtests.unix.in
            ${CMAKE_BINARY_DIR}/run-systemtests
            @ONLY
        )
    elseif(WIN32)
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake_templates/run-systemtests.windows.in
            ${CMAKE_BINARY_DIR}/run-systemtests
            @ONLY
        )
    endif()
    add_custom_target(tests
        DEPENDS systemtests unittests
    )
    add_custom_target(systemtests
        COMMENT "Running system tests"
        COMMAND ${CMAKE_BINARY_DIR}/run-systemtests -vv -s 
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
add_subdirectory(libshared)
add_subdirectory(libcore)
add_subdirectory(jpscore)

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

