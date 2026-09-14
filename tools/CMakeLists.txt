################################################################################
# CLI tools
################################################################################
add_executable(remesh
    remesh.cpp
)

target_link_libraries(remesh PRIVATE
    simulator
    project_options
)

# Live progress counters from the isotropic remesher (implies CGAL_PMP_REMESHING_VERBOSE).
target_compile_definitions(remesh PRIVATE
    CGAL_PMP_REMESHING_VERBOSE_PROGRESS
)
