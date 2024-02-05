add_executable(jps-ui
    src/aabb.cpp
    src/aabb.hpp
    src/disjoint_set.cpp
    src/disjoint_set.hpp
    src/gui.cpp
    src/gui.hpp
    src/main.cpp
    src/mesh.cpp
    src/mesh.hpp
    src/ortho_camera.cpp
    src/ortho_camera.hpp
    src/rendering_mesh.cpp
    src/rendering_mesh.hpp
    src/shader.cpp
    src/shader.hpp
    src/wkt.cpp
    src/wkt.hpp
)

target_link_libraries(jps-ui
    glfw
    OpenGL::GL
    GEOS::geos_c
    CGAL::CGAL
    imgui
    imgui_filedialog
    glm
    glad
    fmt
)

target_compile_definitions(jps-ui PUBLIC
    GL_SILENCE_DEPRECATION
    GLFW_INCLUDE_NONE
)

################################################################################
# jps-ui unit tests
################################################################################
if (BUILD_TESTS)
    add_executable(jps-ui-tests
        test/mesh_test.cpp
        src/aabb.cpp
        src/aabb.hpp
        src/disjoint_set.cpp
        src/disjoint_set.hpp
        src/gui.cpp
        src/gui.hpp
        src/mesh.cpp
        src/mesh.hpp
        src/ortho_camera.cpp
        src/ortho_camera.hpp
        src/rendering_mesh.cpp
        src/rendering_mesh.hpp
        src/shader.cpp
        src/shader.hpp
        src/wkt.cpp
        src/wkt.hpp
    
    )

    target_link_libraries(jps-ui-tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        glfw
        OpenGL::GL
        GEOS::geos_c
        CGAL::CGAL
        imgui
        imgui_filedialog
        glm
        glad
        fmt
        )

    target_compile_options(jps-ui-tests PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )

    # Also allow the unit test access to the non-public header files of jupedsim_obj
    # This is required to construct some otherwise opqaue types in tests, e.g. ErrorMessage
    target_include_directories(jps-ui-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    set_property(TARGET jps-ui-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
    set_property(TARGET jps-ui-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()
