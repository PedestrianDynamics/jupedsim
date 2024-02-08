add_library(imgui STATIC
    imconfig.h
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_impl_glfw.cpp
    imgui_impl_glfw.h
    imgui_impl_opengl3.cpp
    imgui_impl_opengl3.h
    imgui_impl_opengl3_loader.h
    imgui_internal.h
    imgui_stdlib.cpp
    imgui_stdlib.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h  
    imstb_textedit.h  
    imstb_truetype.h
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(imgui PUBLIC glfw)
