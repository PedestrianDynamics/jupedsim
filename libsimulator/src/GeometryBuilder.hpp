#pragma once

#include "Geometry.hpp"
#include "Point.hpp"

#include <vector>

class GeometryBuilder
{
    using LineLoop = std::vector<Point>;
    std::vector<LineLoop> _accessibleAreas{};
    std::vector<LineLoop> _exclusions{};

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
