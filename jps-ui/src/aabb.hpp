// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <glm/vec2.hpp>
#include <vector>

struct AABB {
    glm::vec2 min;
    glm::vec2 max;

    float Aspect() const;
    float Width() const;
    float Height() const;
    glm::vec2 Center() const;

    bool Contains(const glm::vec2& point) const;
};

AABB CreateFromPoints(const std::vector<glm::vec2>& points);
