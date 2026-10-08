add_library(py_jupedsim MODULE
    agent.cpp
    agent_view.cpp
    anticipation_velocity_model.cpp
    bindings_jupedsim.cpp
    build_info.cpp
    collision_free_speed_model.cpp
    collision_free_speed_model_v2.cpp
    collision_free_speed_model_v3.cpp
    conversion.cpp
    conversion.hpp
    generalized_centrifugal_force_model.cpp
    geometry.cpp
    logging.cpp
    logging.hpp
    python_model.cpp
    python_model.hpp
    routing.cpp
    simulation.cpp
    social_force_model.cpp
    stage.cpp
    temporary/boundary_index.cpp
    trace.cpp
    transition.cpp
    type_casters.hpp
    walkable_surface.cpp
    warp_driver_model.cpp
)

target_include_directories(py_jupedsim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(py_jupedsim
    PRIVATE
        pybind11::module
        pybind11::windows_extras
        simulator
        fmt::fmt
        common
        perfetto
        project_options
)

pybind11_extension(py_jupedsim)

if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(py_jupedsim)
endif()

# staging location of jupedsim python package.
# Put this on your PYTHONPATH to use a dev version of
# jupedsim
set(STAGE ${CMAKE_BINARY_DIR}/stage)

# $<1:...> stops multi-config generators (Visual Studio) from appending a
# per-configuration folder, so the module always lands in the stage.
set_target_properties(py_jupedsim
    PROPERTIES
        CXX_VISIBILITY_PRESET "hidden"
        CUDA_VISIBILITY_PRESET "hidden"
        LIBRARY_OUTPUT_DIRECTORY $<1:${STAGE}/jupedsim>
)

set(PKG_SRC ${CMAKE_SOURCE_DIR}/python_modules/jupedsim/jupedsim)

# Mirror jupedsim python package to build tree
# Note: package folders are created in place and individual files are symlinked
# because we build the py_jupedsim.so into this tree. If we would link the whole folder
# py_jupedsim.so would end up in the source tree.
# Files are copied instead where symlinks cannot be created (Windows without
# developer mode); such copies are refreshed on the next configure only.
file(GLOB_RECURSE PKG_FILES CONFIGURE_DEPENDS RELATIVE ${PKG_SRC} ${PKG_SRC}/*)
list(FILTER PKG_FILES EXCLUDE REGEX "(^|/)__pycache__/")
foreach(f IN LISTS PKG_FILES)
    get_filename_component(d ${STAGE}/jupedsim/${f} DIRECTORY)
    file(MAKE_DIRECTORY ${d})
    file(CREATE_LINK ${PKG_SRC}/${f} ${STAGE}/jupedsim/${f} COPY_ON_ERROR SYMBOLIC)
endforeach()

# Stubs are generated directly into the stage. Build directories created
# before that linked this folder to a separate stubs tree; drop the link.
set(STUB_DIR ${STAGE}/jupedsim/py_jupedsim)
if(IS_SYMLINK ${STUB_DIR})
    file(REMOVE ${STUB_DIR})
endif()

# Stub generation imports the built module, so the build has to be able to run
# it: wheels are built natively for each architecture, not cross-compiled.
# NOTE: there is one file per module we export from the bindings
set(STUB_FILES
    ${STUB_DIR}/__init__.pyi
    ${STUB_DIR}/buildinfo.pyi
)
# The module is found via the working directory (python -m puts it first on
# sys.path), not PYTHONPATH: pip's isolated build exposes the build requirements
# through PYTHONPATH, so overriding it would hide pybind11-stubgen and shapely.
# The stub folder is cleared first: stubgen only writes the current modules, so
# stubs of removed submodules would linger and fail the stubtest system test.
add_custom_command(
    OUTPUT ${STUB_FILES}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${STUB_DIR}
    COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen jupedsim.py_jupedsim
            -o ${STAGE} --exit-code
    WORKING_DIRECTORY ${STAGE}
    DEPENDS py_jupedsim
    COMMENT "Generating stubs for jupedsim.py_jupedsim")
add_custom_target(py_jupedsim_stubs ALL DEPENDS ${STUB_FILES})

# Wheel contents. scikit-build-core installs only the "python" component; the
# pure Python package is added via wheel.packages in pyproject.toml.
install(TARGETS py_jupedsim LIBRARY DESTINATION jupedsim COMPONENT python)
install(FILES ${STUB_FILES} DESTINATION jupedsim/py_jupedsim COMPONENT python)
