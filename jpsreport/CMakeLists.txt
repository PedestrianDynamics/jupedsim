# # some options
# # -DCMAKE_CXX_COMPILER=clang++   -DCMAKE_C_COMPILER:STRING=clang
# # -DCMAKE_BUILD_TYPE=Debug (default Release)
# # -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# # -DBUILD_TESTING=ON (default OFF)
# cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
# set(JPSREPORT_MAJOR_VERSION 0)
# set(JPSREPORT_MINOR_VERSION 8)
# set(JPSREPORT_PATCH_VERSION 4)
# set(JPSREPORT_VERSION
#   ${JPSREPORT_MAJOR_VERSION}.${JPSREPORT_MINOR_VERSION}.${JPSREPORT_PATCH_VERSION})
# message( STATUS "JPSREPORT_VERSION: " ${JPSREPORT_VERSION} )
# project(JPSreport VERSION ${JPSREPORT_VERSION} LANGUAGES CXX)
# #set (CMAKE_CXX_STANDARD 17)
# # Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
# if (POLICY CMP0025)
#   cmake_policy(SET CMP0025 NEW)
# endif ()

# set(CMAKE_LEGACY_CYGWIN_WIN32 0)

# IF (POLICY CMP0054)
#   cmake_policy(SET CMP0054 NEW)
# ENDIF (POLICY CMP0054)
# set(CMAKE_COLOR_MAKEFILE ON)

# set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

# if (Boost_NO_SYSTEM_PATHS)
#   set(Boost_NO_SYSTEM_PATHS ON)
#   set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
#   set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib")
#   set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
#   set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_LIBRARY_DIRS})
# endif (Boost_NO_SYSTEM_PATHS)


# find_package(Git REQUIRED) # no need for this msg. It comes from cmake.findgit()

# find_program(GIT_SCM git DOC "Git version control")
# mark_as_advanced(GIT_SCM)
# find_file(GITDIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
# if (GIT_SCM AND GITDIR)

# # the commit's SHA1, and whether the building workspace was dirty or not
# # describe --match=NeVeRmAtCh --always --tags --abbrev=40 --dirty
# execute_process(COMMAND
#   "${GIT_EXECUTABLE}" --no-pager describe --tags --always --dirty
#   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#   OUTPUT_VARIABLE GIT_SHA1
#   ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# # branch
# execute_process(
#   COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
#   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#   OUTPUT_VARIABLE GIT_BRANCH
#   OUTPUT_STRIP_TRAILING_WHITESPACE
# )

# # the date of the commit
# execute_process(COMMAND
#   "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
#   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#   OUTPUT_VARIABLE GIT_DATE
#   ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# # the subject of the commit
# execute_process(COMMAND
#   "${GIT_EXECUTABLE}" log -1 --format=%s
#   WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#   OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
#   ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# string(REGEX REPLACE "\#"
#        "" GIT_COMMIT_SUBJECT
#        ${GIT_COMMIT_SUBJECT})

# add_definitions("-DGIT_COMMIT_HASH=\"${GIT_SHA1}\"")
# add_definitions("-DGIT_COMMIT_DATE=\"${GIT_DATE}\"")
# add_definitions("-DGIT_COMMIT_SUBJECT=\"${GIT_COMMIT_SUBJECT}\"")
# add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")

# else()
#     message(STATUS "Not in a git repo")
# endif()


#set(EXECUTABLE_OUTPUT_PATH "../")
#INCLUDE_DIRECTORIES("./")
# message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR} )
# message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR} )
# message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
# set(CMAKE_TEST_DIR ${CMAKE_SOURCE_DIR}/Utest)


# add_definitions("-DJPSREPORT_VERSION=\"${JPSREPORT_VERSION}\"")
# if(NOT CMAKE_BUILD_TYPE)
#   set (CMAKE_BUILD_TYPE Release)
# endif(NOT CMAKE_BUILD_TYPE)
# message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )

# if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
#   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# endif()

# if(NOT EXECUTABLE_OUTPUT_PATH)
#   set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
# endif()

# set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}")


# message( STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
# message( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
# message( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )

# if(NOT BUILD_TESTING)
#   set(BUILD_TESTING OFF) # test units are generated.
# endif(NOT BUILD_TESTING)
# message( STATUS "BUILD_TESTING: " ${BUILD_TESTING} )

# if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
# # To generate a compilation database "compilation_commands.json" for clang_check
#   set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# endif(NOT CMAKE_EXPORT_COMPILE_COMMANDS)



# if(NOT DEFINED PROCESSOR_COUNT)
#   # Unknown:
#   set(PROCESSOR_COUNT 1)

#   # Linux:
#   set(cpuinfo_file "/proc/cpuinfo")
#   if(EXISTS "${cpuinfo_file}")
#     file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
#     list(LENGTH procs PROCESSOR_COUNT)
#   endif(EXISTS "${cpuinfo_file}")

#   # Windows:
#   if(WIN32)
#     set(PROCESSOR_COUNT "$ENV{NUMBER_OF_PROCESSORS}")
#   endif(WIN32)
# endif(NOT DEFINED PROCESSOR_COUNT)

# if(PROCESSOR_COUNT)
#   # add 1 should be magic! http://www.kitware.com/blog/home/post/63
#   #math(EXPR PROCESSOR_COUNT "${PROCESSOR_COUNT} + 1")
#   message( STATUS "PROCESSOR_COUNT " ${PROCESSOR_COUNT})
#   set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")
# endif(PROCESSOR_COUNT)

# find the correct OpenMP flag
# FIND_PACKAGE(OpenMP)
# if(OPENMP_FOUND)
#   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
# else(OPENMP_FOUND)
#   if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
#     # somehow find_package(openmp) does not work properly with clang
#   else(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     message( STATUS "Disabling OpenMP support" )
#   endif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# endif(OPENMP_FOUND)

# test all cpp-files in Utest
# if(BUILD_TESTING)
#   file(GLOB test_files "${CMAKE_TEST_DIR}/*.cpp")
# #  file(GLOB test_py_files "${CMAKE_TEST_DIR}/*/runtest*.py")
#   file(GLOB_RECURSE  test_py_files "${CMAKE_TEST_DIR}/*runtest_*.py")
# endif(BUILD_TESTING)

set(methods
    methods/VoronoiDiagram.cpp
    methods/PedData.cpp
    methods/Method_A.cpp
    methods/Method_B.cpp
    methods/Method_C.cpp
    methods/Method_D.cpp
    methods/Method_I.cpp
    methods/Method_J.cpp
)

set(source_files
    Analysis.cpp
    IO/OutputHandler.cpp
    general/ArgumentParser.cpp
    geometry/Building.cpp
    geometry/Line.cpp
    geometry/Point.cpp
    geometry/Transition.cpp
    geometry/Crossing.cpp
    geometry/NavLine.cpp
    geometry/Room.cpp
    geometry/Wall.cpp
    geometry/Hline.cpp
    geometry/Obstacle.cpp
    geometry/SubRoom.cpp
    geometry/Goal.cpp
)

set (header_files
    Analysis.h
    methods/MeasurementArea.h
    methods/VoronoiDiagram.h
    methods/PedData.h
    methods/Method_A.h
    methods/Method_B.h
    methods/Method_C.h
    methods/Method_D.h
    methods/Method_I.h
    methods/Method_J.h
    IO/OutputHandler.h
    general/ArgumentParser.h
    general/Macros.h
    geometry/Building.h
    geometry/Line.h
    geometry/Point.h
    geometry/Transition.h
    geometry/Crossing.h
    geometry/NavLine.h
    geometry/Room.h
    geometry/Wall.h
    geometry/Hline.h
    geometry/Obstacle.h
    geometry/SubRoom.h
    geometry/Goal.h
)


#--------------------
#if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
#  include(CheckCXXCompilerFlag)
#  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
#  if(COMPILER_SUPPORTS_CXX11)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#    message(STATUS "Checking for C++11 compiler - available")
#  elseif(COMPILER_SUPPORTS_CXX0X)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#    message(STATUS "Checking for C++0x compiler - available")
#  else()
#    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no c++11 support. Please use a #different C++ compiler.")
#  endif()
#endif()
#---------------------
# if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#   message(STATUS "Using Clang " ${CMAKE_CXX_COMPILER_VERSION})
# endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# if(CMAKE_COMPILER_IS_GNUCXX)
#   message( STATUS "Using g++ " ${CMAKE_CXX_COMPILER_VERSION})
# endif(CMAKE_COMPILER_IS_GNUCXX)

# if(CMAKE_COMPILER_IS_GNUCXX)
#   message( STATUS "Set compiler flags (g++)" )
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-unused-local-typedefs")
#   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wno-unused-local-typedefs")
# endif(CMAKE_COMPILER_IS_GNUCXX)
# if(CMAKE_BUILD_TYPE MATCHES Debug)
#   message(STATUS "Debug flags: " ${CMAKE_CXX_FLAGS_DEBUG} )
# endif()
# if(CMAKE_BUILD_TYPE MATCHES Release)
#   message(STATUS "Release flags: " ${CMAKE_CXX_FLAGS_RELEASE} -static )
# endif()

# if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE MATCHES Debug)
#   set(WITH_COVERAGE TRUE)
#   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
#   set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#   message(STATUS "Debug flags for coverage: " ${CMAKE_CXX_FLAGS_DEBUG} )
# endif(CMAKE_COMPILER_IS_GNUCXX  AND CMAKE_BUILD_TYPE MATCHES Debug)

# if (Boost_NO_SYSTEM_PATHS)
#         set(Boost_NO_SYSTEM_PATHS ON)
#         set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
#         set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib")
#         set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
#         set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${BOOST_LIBRARY_DIRS})
# endif (Boost_NO_SYSTEM_PATHS)
# if(MSVC)
#   find_package(Boost QUIET)
# else()
#   find_package(Boost REQUIRED)
# endif()
# if (Boost_FOUND)
# #        set(Boost_USE_STATIC_LIBS ON)
# #        set(Boost_USE_STATIC_RUNTIME ON)

#         if(WIN32)
#                 add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
#         endif()
#         message(STATUS "BOOST FOUND: " ${Boost_LIB_VERSION})
#         message(STATUS "Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
#         message(STATUS "Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR})
#         message(STATUS "Boost_LIB_VERSION: " ${Boost_LIB_VERSION})
#         message(STATUS "Boost_libraries: " ${Boost_LIBRARIES})
#         link_directories(${Boost_LIBRARY_DIRS})
#         include_directories(SYSTEM ${Boost_INCLUDE_DIR})
#         # suppress warnings in boost libraries with attribute SYSTEM
#         #include_directories(SYSTEM ${Boost_INCLUDE_DIR})
#         else()
#     message(WARNING "BOOST NOT FOUND")
# endif()

#if (WIN32)
#find_library (PSAPI Psapi PATH_SUFFIXES "x64")
#message (STATUS "PSAPI: ${PSAPI}")
#endif()

add_library(report STATIC
    ${source_files}
    ${methods}
    Boost::boost
)

target_link_libraries(report PUBLIC
    tinyxml
    fs
)

target_compile_definitions(report PUBLIC
    PYTHON="${PYTHON_EXECUTABLE}"
    PYTHON_VERSION="${PYTHON_VERSION_STRING}"
)

add_executable(jpsreport
    main.cpp
)

target_link_libraries(jpsreport
    report
)


# # ----------------------------- cTest ------------------------------------------
# if(BUILD_TESTING)
#   SET(BUILD_TEST_TIMEOUT 10800   CACHE STRING "Global timeout on all tests (seconds).")
#   SET(DART_TESTING_TIMEOUT "${BUILD_TEST_TIMEOUT}" CACHE STRING "" FORCE)
#   message(STATUS "Timeout: " ${BUILD_TEST_TIMEOUT} " s"  )
#   enable_testing() #adds another build target, which is test for Makefile generators
#   include(CTest) #adding Dart support

#   #test if code compiles and runs default setting. Takes about 30 seconds
#   add_test (jpsreport_compile ${CMAKE_CTEST_COMMAND}
#   --build-and-test "${CMAKE_SOURCE_DIR}" "${EXECUTABLE_OUTPUT_PATH}" #"${CMAKE_BINARY_DIR}"
#   --build-generator ${CMAKE_GENERATOR}
#   --build-makeprogram ${CMAKE_MAKE_PROGRAM} -j${PROCESSOR_COUNT}
#   --build-two-config
#   --build-exe-dir ${EXECUTABLE_OUTPUT_PATH}  # todo wo soll der exe hin?: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#   --build-project JPSreport
#   --test-command jpsreport --ini=${CMAKE_TEST_DIR}/files/input_UO_180.xml
#   )




#   #  set(Python_ADDITIONAL_VERSIONS 3.0)
#   find_package(PythonInterp)
#   if(PYTHONINTERP_FOUND)
#   message(STATUS "Found Python: " ${PYTHON_EXECUTABLE} " (${PYTHON_VERSION_STRING})")
#     foreach (test_src ${test_py_files})
#       GET_FILENAME_COMPONENT(test ${test_src} NAME_WE)
#      message (STATUS "Add python test: " ${test} " | test_src: " ${test_src})
#        add_test(NAME ${test} COMMAND ${PYTHON_EXECUTABLE} ${test_src})
#     endforeach(test_src ${test_py_files})
#     message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
#   endif(PYTHONINTERP_FOUND)
# endif(BUILD_TESTING)

# # ----------- CPACK
# set(DEMOS_DIR "demos")
# set(DEMO1_DIR "${DEMOS_DIR}/T-Junction")
# set(DEMO2_DIR "${DEMOS_DIR}/bottleneck")
# set(SCRIPTS_DIR "scripts")
# set(NOTEBOOK "${CMAKE_SOURCE_DIR}/${DEMOS_DIR}/HowTo.ipynb")

# file(GLOB CT_FILES "${CMAKE_SOURCE_DIR}/${DEMO1_DIR}/*")
# file(GLOB CT_FILES2 "${CMAKE_SOURCE_DIR}/${DEMO2_DIR}/*")
# file(GLOB SCRIPT_FILES "${CMAKE_SOURCE_DIR}/${SCRIPTS_DIR}/*")

# install(TARGETS jpsreport
#         DESTINATION bin
#         COMPONENT applications)

# install(DIRECTORY DESTINATION ${DEMO1_DIR})
# install(DIRECTORY DESTINATION ${DEMO2_DIR})

# # notebook
# install(FILES ${NOTEBOOK}
#         DESTINATION ${DEMOS_DIR}
#         COMPONENT demos)

# install(FILES ${CT_FILES}
#         DESTINATION ${DEMO1_DIR}
#         COMPONENT demos)

# install(FILES ${CT_FILES2}
#         DESTINATION ${DEMO2_DIR}
#         COMPONENT demos)

# install(FILES ${SCRIPT_FILES}
#         DESTINATION ${SCRIPTS_DIR}
#     COMPONENT scripts)


# if(WIN32 AND NOT UNIX)
#         message(STATUS "Package generation - Windows")
#         set(CPACK_GENERATOR "NSIS")
#         find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
#         if(NSIS_PATH)
#         set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
#         message(STATUS "   + NSIS                                 YES ")
#         else(NSIS_PATH)
#         message(STATUS "   + NSIS                                 NO ")
#         endif(NSIS_PATH)
#         set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
#         include(InstallRequiredSystemLibraries)
#         #message(STATUS "System Dependencies: " ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
#         install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
#         DESTINATION sys
#         COMPONENT applications)

# elseif(APPLE)
# set(DEFAULT_CPACK_GENERATOR "DragNDrop")
# endif()
# set(CPACK_COMPONENTS_ALL applications)
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
# set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/LICENSE")
# set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/jpsreport.ico")
# set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/forms/jpsreport.ico")
# set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
# set(CPACK_NSIS_MODIFY_PATH ON)
# set(CPACK_COMPONENTS_ALL applications demos scripts)
# set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "jpsreport")
# set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "demo files" "Scripts")
# set(CPACK_COMPONENT_DATA_FILES_DESCRIPTION "demo files to get started" "scripts")

# SET(CPACK_PACKAGE_DESCRIPTION "Simulation core of JuPedSim")
# SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The simulation core of JuPedSim, a framework for simulation and analysis of pedestrian dynamics")
# SET(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
# SET(CPACK_PACKAGE_NAME "JPSreport")
# SET(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")
# SET(CPACK_PACKAGE_VERSION_MAJOR "${JPSREPORT_MAJOR_VERSION}")
# SET(CPACK_PACKAGE_VERSION_MINOR "${JPSREPORT_MINOR_VERSION}")
# SET(CPACK_PACKAGE_VERSION_PATCH "${JPSREPORT_PATCH_VERSION}")
# SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${JPSREPORT_VERSION}")
# SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${JPSREPORT_VERSION}")

# INCLUDE(CPack)
