// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "aabb.hpp"

float AABB::Aspect() const
{
    const auto diff = max - min;
    return diff.x / diff.y;
}

float AABB::Width() const
{
    return max.x - min.x;
}

float AABB::Height() const
{
    return max.y - min.y;
}
glm::vec2 AABB::Center() const
{
    return min + (max - min) / 2.0f;
}

bool AABB::Contains(const glm::vec2& p) const
{
    return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
}

AABB CreateFromPoints(const std::vector<glm::vec2>& points)
{
    float xMin = std::numeric_limits<float>::max();
    float xMax = std::numeric_limits<float>::lowest();
    float yMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::lowest();

    for(const auto& p : points) {
        xMin = std::min(xMin, p.x);
        xMax = std::max(xMax, p.x);
        yMin = std::min(yMin, p.y);
        yMax = std::max(yMax, p.y);
    }

    return AABB{{xMin, yMin}, {xMax, yMax}};
}