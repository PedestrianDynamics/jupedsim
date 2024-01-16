add_executable(jps-ui
    src/aabb.cpp
    src/aabb.hpp
    src/gui.cpp
    src/gui.hpp
    src/main.cpp
    src/ortho_camera.cpp
    src/ortho_camera.hpp
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
)

target_compile_definitions(jps-ui PUBLIC
    GL_SILENCE_DEPRECATION
    GLFW_INCLUDE_GLCOREARB
)

