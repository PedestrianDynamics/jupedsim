################################################################################
# libfmt
################################################################################
set(FMT_INSTALL OFF)
add_subdirectory(fmt-10.2.1)
set_target_properties(fmt PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    SYSTEM ON
)

################################################################################
# boost
################################################################################
find_package(Boost 1.82
    REQUIRED
    CONFIG
    NO_DEFAULT_PATH
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/boost-1.82.0
)
set_target_properties(Boost::boost PROPERTIES
	IMPORTED_GLOBAL TRUE
	SYSTEM TRUE
)
################################################################################
# threading
################################################################################
find_package(Threads REQUIRED)

################################################################################
# CGAL
################################################################################
# Silence CGAL CMake warning. No example data for CGAL will be loaded anyways
set(CGAL_DATA_DIR "none")
set(WITH_CGAL_ImageIO OFF)
set(WITH_CGAL_Qt5 OFF)
set(CGAL_DISABLE_GMP ON)
find_package(CGAL 6.0
    REQUIRED
    CONFIG
    NO_DEFAULT_PATH
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/cgal-6.0.1
)

################################################################################
# Python Interpreter / CPython
################################################################################
set(Python_FIND_VIRTUALENV "FIRST")
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_SIMPLE_GIL_MANAGEMENT ON)
set(PYBIND11_NUMPY_1_ONLY ON)
set(PYBIND11_FINDPYTHON ON)
add_subdirectory(pybind11-2.12.0)
set(Python_EXECUTABLE ${Python_EXECUTABLE} PARENT_SCOPE)

################################################################################
# Googletest
################################################################################
if(BUILD_TESTS)
    add_subdirectory(googletest-1.14.0)
endif()

################################################################################
# Googlebenchmarks
################################################################################
if (BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF)
    set(BENCHMARK_ENABLE_INSTALL OFF)
    set(BENCHMARK_ENABLE_WERROR OFF)
    set(BENCHMARK_INSTALL_DOCS OFF)
    add_subdirectory(benchmark-1.8.2)
endif ()

################################################################################
# glm
################################################################################
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
add_subdirectory(glm-1.0.1)
