// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "mesh.hpp"
#include "shader.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class RenderingMesh
{
    /// 0: Solid pass
    /// 1: Edge pass
    GLuint vaos[2]{};
    /// 0: vertices
    /// 1: triangle indices (GL_TRIANGLES)
    /// 2: edge indices (GL_LINES)
    GLuint buffer[3]{};
    /// 0: triangle index count
    /// 1: lines index count
    GLuint index_count[2]{};

public:
    RenderingMesh(const Mesh& data);
    ~RenderingMesh();
    void Draw(Shader& shader) const;
};
