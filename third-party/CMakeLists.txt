################################################################################
# libfmt
################################################################################
set(FMT_INSTALL OFF)
add_subdirectory(fmt-10.1.1)
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
set_target_properties(Boost::boost PROPERTIES IMPORTED_GLOBAL TRUE)
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
find_package(CGAL 5.5
    REQUIRED
    CONFIG
    NO_DEFAULT_PATH
    PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/cgal-5.6.0
)

################################################################################
# poly2tri
################################################################################
add_subdirectory(poly2tri)

################################################################################
# Python Interpreter / CPython
################################################################################
set(Python_FIND_VIRTUALENV "FIRST")
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
add_subdirectory(pybind11-2.11.1)
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
endif()

################################################################################
# DearImGui / ImGuiFileDialog
################################################################################
if(BUILD_JPS_UI)
    set(BUILD_SHARED_LIBS_BEFORE ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)
    find_package(OpenGL REQUIRED)
    set_target_properties(OpenGL::GL PROPERTIES IMPORTED_GLOBAL TRUE)
    add_subdirectory(dearimgui/v1.90.1)
    add_subdirectory(imgui_filedialog)
    add_subdirectory(geos-3.12.1)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_INSTALL OFF)
    add_subdirectory(glfw-3.3.9)
    #TODO(kkratz): this glm version is the latest commit on the v1.0.0 release
    # branch, we should switch to the official release at some time
    add_subdirectory(glm-glm-v1)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_BEFORE})
endif()
