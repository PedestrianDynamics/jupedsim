add_library(imgui_filedialog STATIC
    v0.6.6.1/ImGuiFileDialog.cpp
    v0.6.6.1/ImGuiFileDialog.h
    v0.6.6.1/ImGuiFileDialogConfig.h
)

target_include_directories(imgui_filedialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/v0.6.6.1)

target_link_libraries(imgui_filedialog PUBLIC imgui)
