// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "rendering_mesh.hpp"
#include "mesh.hpp"

RenderingMesh::RenderingMesh(const Mesh& data)
{
    glGenVertexArrays(2, vaos);
    glGenBuffers(3, buffer);

    const auto vertices = data.FVertices();
    const auto triangle_indices = data.TriangleIndices();
    index_count[0] = triangle_indices.size();

    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(decltype(vertices)::value_type),
        reinterpret_cast<const GLvoid*>(vertices.data()),
        GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        index_count[0] * sizeof(decltype(triangle_indices)::value_type),
        reinterpret_cast<const GLvoid*>(triangle_indices.data()),
        GL_STATIC_DRAW);

    const auto segment_indices = data.SegmentIndices();
    index_count[1] = segment_indices.size();

    glBindVertexArray(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[2]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        index_count[1] * sizeof(decltype(segment_indices)::value_type),
        reinterpret_cast<const GLvoid*>(segment_indices.data()),
        GL_STATIC_DRAW);
}

RenderingMesh::~RenderingMesh()
{
    glDeleteBuffers(3, buffer);
    glDeleteBuffers(2, vaos);
}

void RenderingMesh::Draw(Shader& shader) const
{
    shader.Activate();

    shader.SetUniform("color", glm::vec4(0.0f, 0.0f, 0.75f, 1.0f));
    glBindVertexArray(vaos[0]);
    glDrawElements(GL_TRIANGLES, index_count[0], GL_UNSIGNED_SHORT, nullptr);

    shader.SetUniform("color", glm::vec4(0.0f, 0.75f, 0.0f, 1.0f));
    glBindVertexArray(vaos[1]);
    glDrawElements(GL_LINES, index_count[1], GL_UNSIGNED_SHORT, nullptr);
}
