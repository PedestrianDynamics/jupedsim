// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry.hpp"
#include "Polygon.hpp"

#include <vector>

class GeometryBuilder
{
    std::vector<Polygon> _accessibleAreas{};
    std::vector<Polygon> _exclusions{};

public:
    GeometryBuilder() = default;
    ~GeometryBuilder() = default;
    GeometryBuilder(const GeometryBuilder& other) = delete;
    GeometryBuilder& operator=(const GeometryBuilder& other) = delete;
    GeometryBuilder(GeometryBuilder&& other) = delete;
    GeometryBuilder& operator=(GeometryBuilder&& other) = delete;

    GeometryBuilder& AddAccessibleArea(const std::vector<Point>& lineLoop);
    GeometryBuilder& ExcludeFromAccessibleArea(const std::vector<Point>& lineLoop);
    Geometry Build();
};
