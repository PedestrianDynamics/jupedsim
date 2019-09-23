# Documentation: Some useful options:
# -DCMAKE_CXX_COMPILER=clang++
# -DCMAKE_BUILD_TYPE=Debug (default Release)
# -DCMAKE_VERBOSE_MAKEFILE=ON (default OFF)
# -DBUILD_TESTING=ON (default OFF) for python tests
# -DBUILD_CPPUNIT_TEST=ON (default OFF) for unit tests
# -D Boost_NO_SYSTEM_PATHS=true (default false) -D BOOST_ROOT=PATH_where_to_find_boost
# -D AIROUTER=true (default false)
# -D JPSFIRE=ON (default OFF)
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

set(JPSFIRE OFF CACHE BOOL "Build with jpsfire support")
print_var(JPSFIRE)

set(AIROUTER OFF CACHE BOOL "Build with AI router support")
print_var(AIROUTER)

set(USE_OPENMP ON CACHE BOOL "Build with OpenMP")
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
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
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

# CMakes find_pacakge has no support for generator expressions hence we have to
# do this the hard way
if(${CMAKE_SYSTEM} MATCHES "Darwin")
    list(APPEND BOOST_COMPONENTS
        filesystem
    )
endif()
find_package(Boost 1.65 REQUIRED ${BOOST_COMPONENTS})

# boost filesystem / std::filesystem
add_library(fs INTERFACE)
target_link_libraries(fs INTERFACE
    $<$<PLATFORM_ID:Darwin>:Boost::filesystem>
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
        -Wno-unknown-pragmas
    )
endif()

if(AIROUTER)
    set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE ON)
    find_package(CGAL REQUIRED)
    list(APPEND COMMON_COMPILE_OPTIONS
        # CGAL package puts a gcc only optimization flag on the link line for
        # all targets that use CGAL. Since we do not want to see an waring for
        # an unknown optimization flag on clang we silence this warning.
        $<$<CXX_COMPILER_ID:Clang>:-Wno-ignored-optimization-argument>
    )
endif()


find_package(Threads REQUIRED)

if(BUILD_TESTING)
    find_package(PythonInterp 3 REQUIRED)
endif()

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

    # the subject of the commit
    execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    # remove # from subject
    string(REGEX REPLACE "\#"
       "" GIT_COMMIT_SUBJECT
       ${GIT_COMMIT_SUBJECT})

else()
    message(STATUS "Not in a git repo")
    set(GIT_SHA1 "UNKNONW")
    set(GIT_DATE "UNKNONW")
    set(GIT_COMMIT_SUBJECT "UNKNONW")
    set(GIT_BRANCH "UNKNONW")
endif()

list(APPEND GIT_BUILD_INFO_DEFINES
    GIT_COMMIT_HASH="${GIT_SHA1}"
    GIT_COMMIT_DATE="${GIT_DATE}"
    GIT_COMMIT_SUBJECT="${GIT_COMMIT_SUBJECT}"
    GIT_BRANCH="${GIT_BRANCH}"
)

################################################################################
# Documentation
################################################################################
if(BUILD_DOC)
    find_package(Doxygen REQUIRED)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif(BUILD_DOC)

################################################################################
# Build libcore
################################################################################
set(source_files
    events/Event.cpp
    events/EventManager.cpp
    forms/jpscore.rc
    general/ArgumentParser.cpp
    general/Logger.cpp
    general/Filesystem.cpp
    geometry/Building.cpp
    geometry/Crossing.cpp
    geometry/Goal.cpp
    geometry/GoalManager.cpp
    geometry/Hline.cpp
    geometry/Line.cpp
    geometry/NavLine.cpp
    geometry/Obstacle.cpp
    geometry/Point.cpp
    geometry/Room.cpp
    geometry/SubRoom.cpp
    geometry/Transition.cpp
    geometry/WaitingArea.cpp
    geometry/Wall.cpp
    IO/GeoFileParser.cpp
    IO/IniFileParser.cpp
    IO/IODispatcher.cpp
    IO/OutputHandler.cpp
    IO/PedDistributionParser.cpp
    IO/progress_bar.cpp
    IO/TraVisToClient.cpp
    math/ForceModel.cpp
    math/GCFMModel.cpp
    math/GompertzModel.cpp
    math/GradientModel.cpp
    math/KrauszModel.cpp
    math/Mathematics.cpp
    math/ODESolver.cpp
    math/OperationalModel.cpp
    math/VelocityModel.cpp
    mpi/LCGrid.cpp
    pedestrian/AgentsParameters.cpp
    pedestrian/AgentsQueue.cpp
    pedestrian/AgentsSource.cpp
    pedestrian/AgentsSourcesManager.cpp
    pedestrian/Ellipse.cpp
    pedestrian/Knowledge.cpp
    pedestrian/PedDistributor.cpp
    pedestrian/Pedestrian.cpp
    pedestrian/Pedestrian.cpp
    pedestrian/StartDistribution.cpp
    direction/DirectionManager.cpp
    direction/waiting/WaitingMiddle.cpp
        direction/waiting/WaitingRandom.cpp
    direction/waiting/WaitingStrategy.cpp
    direction/walking/DirectionFloorfield.cpp
    direction/walking/DirectionGeneral.cpp
    direction/walking/DirectionInRangeBottleneck.cpp
    direction/walking/DirectionLocalFloorfield.cpp
    direction/walking/DirectionMiddlePoint.cpp
    direction/walking/DirectionMinSeperationShorterLine.cpp
    direction/walking/DirectionStrategy.cpp
    direction/walking/DirectionSubLocalFloorfield.cpp
    direction/walking/DirectionTrain.cpp
    router/ff_router/ffRouter.cpp
    router/ff_router/FloorfieldViaFM.cpp
    router/ff_router/UnivFFviaFM.cpp
    router/global_shortest/AccessPoint.cpp
    router/global_shortest/DTriangulation.cpp
    router/global_shortest/GlobalRouter.cpp
    router/quickest/QuickestPathRouter.cpp
    router/Router.cpp
    router/RoutingEngine.cpp
    router/smoke_router/Brain.cpp
    router/smoke_router/BrainStorage.cpp
    router/smoke_router/cognitiveMap/associations.cpp
    router/smoke_router/cognitiveMap/cogmapoutputhandler.cpp
    router/smoke_router/cognitiveMap/cognitivemap.cpp
    router/smoke_router/cognitiveMap/connection.cpp
    router/smoke_router/cognitiveMap/internnavigationnetwork.cpp
    router/smoke_router/cognitiveMap/landmark.cpp
    router/smoke_router/cognitiveMap/landmarknetwork.cpp
    router/smoke_router/cognitiveMap/region.cpp
    router/smoke_router/cognitiveMap/youareherepointer.cpp
    router/smoke_router/GraphNetwork.cpp
    router/smoke_router/navigation_graph/GraphEdge.cpp
    router/smoke_router/navigation_graph/GraphVertex.cpp
    router/smoke_router/NavigationGraph.cpp
    router/smoke_router/sensor/AbstractSensor.cpp
    router/smoke_router/sensor/DiscoverDoorsSensor.cpp
    router/smoke_router/sensor/JamSensor.cpp
    router/smoke_router/sensor/LastDestinationsSensor.cpp
    router/smoke_router/sensor/locater.cpp
    router/smoke_router/sensor/RoomToFloorSensor.cpp
    router/smoke_router/sensor/SensorManager.cpp
    router/smoke_router/SmokeRouter.cpp
    Simulation.cpp
    voronoi-boost/VoronoiPositionGenerator.cpp
)
set(header_files
    events/Event.h
    events/EventManager.h
    general/ArgumentParser.h
    general/Compiler.h
    general/Configuration.h
    general/Filesystem.h
    general/Logger.h
    general/Macros.h
    general/randomnumbergenerator.h
    geometry/Building.h
    geometry/Crossing.h
    geometry/Goal.h
    geometry/GoalManager.h
    geometry/Hline.h
    geometry/Line.h
    geometry/NavLine.h
    geometry/Obstacle.h
    geometry/Point.h
    geometry/Room.h
    geometry/SubRoom.h
    geometry/Transition.h
    geometry/WaitingArea.h
    geometry/Wall.h
    IO/GeoFileParser.h
    IO/IniFileParser.h
    IO/IODispatcher.h
    IO/OutputHandler.h
    IO/PedDistributionParser.h
    IO/TraVisToClient.h
    math/ForceModel.h
    math/GCFMModel.h
    math/GompertzModel.h
    math/GradientModel.h
    math/Mathematics.h
    math/ODESolver.h
    math/OperationalModel.h
    math/VelocityModel.h
    mpi/LCGrid.h
    pedestrian/AgentsParameters.h
    pedestrian/AgentsQueue.h
    pedestrian/AgentsSource.h
    pedestrian/AgentsSourcesManager.h
    pedestrian/Ellipse.h
    pedestrian/Knowledge.h
    pedestrian/PedDistributor.h
    pedestrian/Pedestrian.h
    pedestrian/Pedestrian.h
    pedestrian/StartDistribution.h
    direction/DirectionManager.h
    direction/waiting/WaitingMiddle.h
        direction/waiting/WaitingRandom.h
    direction/waiting/WaitingStrategy.h
    direction/walking/DirectionFloorfield.h
    direction/walking/DirectionGeneral.h
    direction/walking/DirectionInRangeBottleneck.h
    direction/walking/DirectionLocalFloorfield.h
    direction/walking/DirectionMiddlePoint.h
    direction/walking/DirectionMinSeperationShorterLine.h
    direction/walking/DirectionStrategy.h
    direction/walking/DirectionSubLocalFloorfield.h
    direction/walking/DirectionTrain.h
    router/ff_router/ffRouter.h
    router/ff_router/FloorfieldViaFM.h
    router/ff_router/UnivFFviaFM.h
    router/global_shortest/AccessPoint.h
    router/global_shortest/DTriangulation.h
    router/global_shortest/GlobalRouter.h
    router/quickest/QuickestPathRouter.h
    router/Router.h
    router/RoutingEngine.h
    router/smoke_router/Brain.h
    router/smoke_router/BrainStorage.h
    router/smoke_router/cognitiveMap/associations.h
    router/smoke_router/cognitiveMap/cogmapoutputhandler.h
    router/smoke_router/cognitiveMap/cognitivemap.h
    router/smoke_router/cognitiveMap/connection.cpp
    router/smoke_router/cognitiveMap/internnavigationnetwork.h
    router/smoke_router/cognitiveMap/landmark.h
    router/smoke_router/cognitiveMap/landmarknetwork.h
    router/smoke_router/cognitiveMap/region.h
    router/smoke_router/cognitiveMap/youareherepointer.h
    router/smoke_router/GraphNetwork.h
    router/smoke_router/navigation_graph/GraphEdge.h
    router/smoke_router/navigation_graph/GraphVertex.h
    router/smoke_router/NavigationGraph.h
    router/smoke_router/sensor/AbstractSensor.h
    router/smoke_router/sensor/DiscoverDoorsSensor.h
    router/smoke_router/sensor/JamSensor.h
    router/smoke_router/sensor/LastDestinationsSensor.h
    router/smoke_router/sensor/locater.h
    router/smoke_router/sensor/RoomToFloorSensor.h
    router/smoke_router/sensor/SensorManager.h
    router/smoke_router/SmokeRouter.h
    voronoi-boost/VoronoiPositionGenerator.h
)
set(JPSFIRE_HDR
    JPSfire/generic/FDSMesh.h
    JPSfire/generic/Knot.h
    JPSfire/generic/FDSMeshStorage.h
    JPSfire/A_smoke_sensor/SmokeSensor.h
    JPSfire/B_walking_speed/WalkingSpeed.h
    JPSfire/C_toxicity_analysis/ToxicityAnalysis.h
    JPSfire/C_toxicity_analysis/ToxicityOutputhandler.h
)
set(JPSFIRE_SRC
    JPSfire/generic/FDSMesh.cpp
    JPSfire/generic/Knot.cpp
    JPSfire/generic/FDSMeshStorage.cpp
    JPSfire/A_smoke_sensor/SmokeSensor.cpp
    JPSfire/B_walking_speed/WalkingSpeed.cpp
    JPSfire/C_toxicity_analysis/ToxicityAnalysis.cpp
    JPSfire/C_toxicity_analysis/ToxicityOutputhandler.cpp
)
set(AIROUTER_SRC
    router/ai_router/AIRouter.cpp
    router/ai_router/BrainStorage.cpp
    router/ai_router/cognitiveMap/associations.cpp
    router/ai_router/cognitiveMap/connection.cpp
    router/ai_router/cognitiveMap/cognitivemap.cpp
    router/ai_router/cognitiveMap/landmark.cpp
    router/ai_router/cognitiveMap/region.cpp
    router/ai_router/cognitiveMap/landmarknetwork.cpp
    router/ai_router/cognitiveMap/internnavigationnetwork.cpp
    router/ai_router/perception/visualsystem.cpp
    router/ai_router/perception/visibleenvironment.cpp
    router/ai_router/perception/cgalgeometry.cpp
    router/ai_router/perception/sign.cpp
    router/ai_router/Cortex.cpp
)
set(AIROUTER_HDR
    router/ai_router/AIRouter.h
    router/ai_router/BrainStorage.h
    router/ai_router/cognitiveMap/associations.h
    router/ai_router/cognitiveMap/connection.cpp
    router/ai_router/cognitiveMap/cognitivemap.h
    router/ai_router/cognitiveMap/landmark.h
    router/ai_router/cognitiveMap/region.h
    router/ai_router/cognitiveMap/landmarknetwork.h
    router/ai_router/cognitiveMap/internnavigationnetwork.h
    router/ai_router/perception/visualsystem.h
    router/ai_router/perception/visibleenvironment.h
    router/ai_router/perception/cgalgeometry.h
    router/ai_router/perception/sign.h
    router/ai_router/Cortex.h
)

add_library(core STATIC
    ${source_files}
    ${header_files}
    $<$<BOOL:${AIROUTER}>:${AIROUTER_SRC}>
    $<$<BOOL:${AIROUTER}>:${AIROUTER_HDR}>
    $<$<BOOL:${JPSFIRE}>:${JPSFIRE_SRC}>
    $<$<BOOL:${JPSFIRE}>:${JPSFIRE_HDR}>
)
target_compile_options(core PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_compile_definitions(core PUBLIC
    $<$<BOOL:${AIROUTER}>:AIROUTER>
    $<$<BOOL:${JPSFIRE}>:JPSFIRE>
    JPSCORE_VERSION="${PROJECT_VERSION}"
    ${GIT_BUILD_INFO_DEFINES}
)
target_link_libraries(core
    Boost::boost
    $<$<BOOL:${USE_OPENMP}>:OpenMP::OpenMP_CXX>
    poly2tri
    tinyxml
    visilibity
    $<$<BOOL:${AIROUTER}>:CGAL::CGAL>
    $<$<BOOL:${JPSFIRE}>:cnpy>
    fs
    spdlog::spdlog
    fmt::fmt
)
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

################################################################################
# Build jspcore executable
################################################################################
add_executable(jpscore
    main.cpp
    ${CMAKE_SOURCE_DIR}/forms/JPScore.ico
)
target_compile_options(jpscore PRIVATE
    ${COMMON_COMPILE_OPTIONS}
)
target_link_libraries(jpscore
    core
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
)

if (WIN32)
    target_link_libraries(jpscore core wsock32)
endif (WIN32)


if(WIN32)
  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT jpscore)
    message(STATUS "set start project for VS")
  endif()
endif()

################################################################################
# Build jspcore_asan executable - jpscore with address sanitizer
################################################################################
if(BUILD_WITH_ASAN)
    get_target_property(core_source core SOURCES)
    get_target_property(core_compile_options core COMPILE_OPTIONS)
    get_target_property(core_compile_definitions core COMPILE_DEFINITIONS)
    get_target_property(core_link_libraries core LINK_LIBRARIES)
    get_target_property(core_include_directories core INCLUDE_DIRECTORIES)

    add_library(core_asan STATIC
        ${core_source}
    )

    target_compile_options(core_asan PRIVATE
        ${core_compile_options}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize=address
    )

    target_compile_definitions(core_asan PRIVATE
        ${core_compile_definitions}
    )

    target_link_libraries(core_asan
        ${core_link_libraries}
    )

  target_include_directories(core_asan PUBLIC
        ${core_include_directories}
    )

    get_target_property(jpscore_source jpscore SOURCES)
    get_target_property(jpscore_compile_options jpscore COMPILE_OPTIONS)
    get_target_property(jpscore_link_libraries jpscore LINK_LIBRARIES)
    list(REMOVE_ITEM jpscore_link_libraries core)
    list(APPEND jpscore_link_libraries core_asan)
    add_executable(jpscore_asan
        ${jpscore_source}
    )

    target_compile_options(jpscore_asan PRIVATE
        ${jpscore_compile_options}
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -fsanitize=address
    )

    target_link_libraries(jpscore_asan
        ${jpscore_link_libraries}
        -fsanitize=address
    )
endif()
################################################################################
# Testing
################################################################################
if(BUILD_CPPUNIT_TEST OR BUILD_TESTING)
    enable_testing()
endif()

################################################################################
# Unit tests
################################################################################
if (BUILD_CPPUNIT_TEST)
    find_package(Catch2 REQUIRED)

    add_executable(unittests
      test/catch2/Main.cpp
      test/catch2/pedestrian/EllipseTest.cpp

      test/catch2/math/MathematicsTest.cpp

      test/catch2/geometry/LineTest.cpp
      test/catch2/geometry/SubRoomTest.cpp
      test/catch2/geometry/RoomTest.cpp
      test/catch2/geometry/PointTest.cpp
      test/catch2/geometry/ObstacleTest.cpp
    )

    target_link_libraries(unittests Catch2::Catch2 core)

    target_compile_options(unittests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    include(CTest)
    include(ParseAndAddCatchTests)
    set(PARSE_CATCH_TESTS_ADD_TARGET_IN_TEST_NAME Off)
    set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS On)
    ParseAndAddCatchTests(unittests)

endif()

################################################################################
# Integration tests
################################################################################
if (BUILD_TESTING)
    list(APPEND clean_geometry_tests
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile1.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile2.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile3.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile4.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile5.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile6.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile7.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile8.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile9-1.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile9-2.xml
        ${CMAKE_SOURCE_DIR}/Utest/test_geometry/clean_geometry/inifile10.xml
    )
    foreach(file ${clean_geometry_tests})
        get_filename_component(test ${file} NAME_WE)
        add_test(
            NAME geometry-${test}
            COMMAND $<TARGET_FILE:jpscore> ${file}
        )
    endforeach()

    file(GLOB_RECURSE test_py_files "${CMAKE_SOURCE_DIR}/Utest/*runtest_*.py")
    foreach (file ${test_py_files})
        get_filename_component(test ${file} NAME_WE)
        add_test(
            NAME ${test}
            COMMAND ${PYTHON_EXECUTABLE} ${file}
        )
    endforeach ()
endif (BUILD_TESTING)

################################################################################
# Install targets
################################################################################
install(TARGETS jpscore
        DESTINATION bin
        COMPONENT applications)
if(WIN32)
    set(CT_DATA_FILE_DIR "demos")
    file(GLOB CT_FILES "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*/*")
    install(FILES ${CT_FILES}
            DESTINATION ${CT_DATA_FILE_DIR}
            COMPONENT demos)
endif()

################################################################################
# Packaging with CPack
################################################################################
IF(UNIX)
    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    SET( CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://jupedsim.org")
    SET(CPACK_DEBIAN_PACKAGE_SUGGESTS, "jpsvis, jpsreport, jpseditor")
    SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
    SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()
if(WIN32 AND NOT UNIX)
    message(STATUS "Package generation - Windows")
    set(CPACK_GENERATOR "NSIS")
    find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
    if(NSIS_PATH)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
        message(STATUS "   + NSIS                                 YES ")
    else(NSIS_PATH)
        message(STATUS "   + NSIS                                 NO ")
    endif(NSIS_PATH)

    #CPACK_NSIS_URL_INFO_ABOUT
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)

    message(STATUS "System Dependencies: " ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
            DESTINATION sys
            COMPONENT applications)
elseif(APPLE)
    set(DEFAULT_CPACK_GENERATOR "DragNDrop")
endif()

set(CPACK_COMPONENTS_ALL applications)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/forms/jpscore.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/forms/jpscore.ico")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_COMPONENTS_ALL applications demos)
set(CPACK_COMPONENT_CTDATA_GROUP "Demos")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "jpscore and dlls")
set(CPACK_COMPONENT_GROUP_DATA_DESCRIPTION "demo files")
set(CPACK_COMPONENT_DATA_FILES_DESCRIPTION "demo files to get started")
SET(CPACK_PACKAGE_DESCRIPTION "Simulation core of JuPedSim")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The simulation core of JuPedSim, a framework for simulation and analysis of pedestrian dynamics")
SET(CPACK_PACKAGE_VENDOR "Forschungszentrum Juelich GmbH")
SET(CPACK_PACKAGE_NAME "jpscore")
SET(CPACK_PACKAGE_CONTACT "m.chraibi@fz-juelich.de")
SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${PROJET_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PROJET_PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJET_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJET_VERSION}")

INCLUDE(CPack)
