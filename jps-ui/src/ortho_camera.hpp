// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "aabb.hpp"
#include "shader.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class OrthoCamera
{
private:
    glm::vec3 eye{0.0f};
    glm::vec3 center{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::mat4x4 view{1.0f};
    glm::mat4x4 projection{1.0f};
    float aspect{1.0f};
    float frustrum_half_width{1.0f};
    bool dirty{true};

public:
    OrthoCamera();
    ~OrthoCamera() = default;

    void CenterOn(AABB bounds);
    void ChangeViewport(float width, float height);
    void Update(Shader& shader);
};
