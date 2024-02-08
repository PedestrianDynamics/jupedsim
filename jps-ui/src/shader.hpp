// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

class Shader
{
private:
    GLuint program{};
    std::unordered_map<std::string, GLuint> uniforms{};

public:
    Shader(const std::string& vertex_code, const std::string& fragment_code);
    ~Shader();

    void Activate();
    void SetUniform(const std::string& name, glm::vec4 val);
    void SetUniform(const std::string& name, glm::mat4x4 val);
};
