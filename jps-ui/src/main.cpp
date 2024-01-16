// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "gui.hpp"
#include "ortho_camera.hpp"
#include "shader.hpp"
#include "wkt.hpp"

#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <geos_c.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <limits>
#include <vector>

using vec3 = glm::dvec3;

static void geos_msg_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct Mesh {
    std::vector<vec3> vertices{};
    std::vector<GLint> indices{};
};

struct ApplicationState {
};

const std::string vertex_shader_code = R"(
    #version 330 core
    layout (location = 0) in vec2 inPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main()
    {
       gl_Position = projection * view * model * vec4(inPos.x, inPos.y, 0.0, 1.0);
    }
)";

const std::string fragment_shader_code = R"(
    #version 330 core
    uniform vec4 color;
    out vec4 outColor;

    void main()
    {
        outColor = color;
    }
)";

int main(int argc, char** argv)
{
    glfwSetErrorCallback(glfw_error_callback);
    initGEOS(geos_msg_handler, geos_msg_handler);
    GLFWwindow* window;

    /* Initialize the library */
    if(!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(640, 480, "Mesh Viewer", NULL, NULL);

    if(!window) {
        glfwTerminate();
        return 1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    OrthoCamera cam{};
    Shader shader(vertex_shader_code, fragment_shader_code);
    shader.Activate();

    shader.SetUniform("model", glm::mat4x4(1.0f));

    Gui gui{};

    /* Loop until the user closes the window */
    while(!glfwWindowShouldClose(window)) {
        /* Poll for and process events */
        glfwPollEvents();
        GLint display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
            gui.clear_color.x * gui.clear_color.w,
            gui.clear_color.y * gui.clear_color.w,
            gui.clear_color.z * gui.clear_color.w,
            gui.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        if(gui.RecenterOnGeometry() && gui.Geometry() != nullptr) {
            cam.CenterOn(gui.Geometry()->Bounds());
        }
        cam.Update(shader);
        if(gui.Geometry()) {
            gui.Geometry()->Draw(shader);
        }
        gui.Draw();

        if(gui.ShouldExit()) {
            glfwSetWindowShouldClose(window, gui.ShouldExit());
        }

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
