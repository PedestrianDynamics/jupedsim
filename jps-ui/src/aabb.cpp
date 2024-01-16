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
