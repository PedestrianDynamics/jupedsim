// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "shader.hpp"

#include <OpenGL/OpenGL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>
#include <stdexcept>
#include <vector>

Shader::Shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code)
{

    const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    const auto vertex_shader_sources = vertex_shader_code.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_sources, nullptr);
    glCompileShader(vertex_shader);

    GLint vertex_shader_compiled{};
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
    if(vertex_shader_compiled != GL_TRUE) {
        GLint log_length{};
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
        std::string error_msg(log_length, '\0');
        glGetShaderInfoLog(vertex_shader, error_msg.size(), nullptr, error_msg.data());

        std::stringstream buf{};
        buf << "Error compiling vertex shader:\n" << error_msg;
        throw std::runtime_error(buf.str());
    }

    const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const auto fragment_shader_sources = fragment_shader_code.c_str();
    glShaderSource(fragment_shader, 1, &fragment_shader_sources, nullptr);
    glCompileShader(fragment_shader);

    GLint fragment_shader_compiled{};
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
    if(fragment_shader_compiled != GL_TRUE) {
        GLint log_length{};
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        std::string error_msg(log_length, '\0');
        glGetShaderInfoLog(fragment_shader, error_msg.size(), nullptr, error_msg.data());
        std::stringstream buf{};
        buf << "Error compiling fragment shader:\n" << error_msg;
        throw std::runtime_error(buf.str());
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint shader_linked{};
    glGetProgramiv(program, GL_LINK_STATUS, &shader_linked);
    if(shader_linked != GL_TRUE) {
        GLint log_length{};
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::string error_msg(log_length, '\0');
        glGetProgramInfoLog(program, error_msg.size(), nullptr, error_msg.data());
        std::stringstream buf{};
        buf << "Error linking shader:\n" << error_msg;
        throw std::runtime_error(buf.str());
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glUseProgram(program);

    GLint max_uniform_id_len{};
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_id_len);
    std::vector<char> buf(max_uniform_id_len, '\0');

    GLint count_uniforms{};
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count_uniforms);

    for(GLuint uniform_index = 0; uniform_index < count_uniforms; ++uniform_index) {
        GLint size{};
        GLenum type{};
        glGetActiveUniform(
            program, uniform_index, max_uniform_id_len, nullptr, &size, &type, buf.data());
        const auto location = glGetUniformLocation(program, buf.data());
        uniforms[std::string(buf.data())] = location;
    }
}

Shader::~Shader()
{
    glDeleteProgram(program);
}

void Shader::Activate()
{
    glUseProgram(program);
}

void Shader::SetUniform(const std::string& name, glm::vec4 val)
{
    glUniform4f(uniforms.at(name), val.r, val.g, val.b, val.a);
}

void Shader::SetUniform(const std::string& name, glm::mat4x4 val)
{
    glUniformMatrix4fv(uniforms.at(name), 1, GL_FALSE, glm::value_ptr(val));
}
