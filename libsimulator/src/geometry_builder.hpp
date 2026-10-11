// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "point.hpp"
#include "polygon.hpp"

#include <vector>

class GeometryBuilder
{
    std::vector<Polygon> _accessible_areas{};
    std::vector<Polygon> _exclusions{};

public:
    GeometryBuilder() = default;
    ~GeometryBuilder() = default;
    GeometryBuilder(const GeometryBuilder& other) = delete;
    GeometryBuilder& operator=(const GeometryBuilder& other) = delete;
    GeometryBuilder(GeometryBuilder&& other) = delete;
    GeometryBuilder& operator=(GeometryBuilder&& other) = delete;

    GeometryBuilder& add_accessible_area(const std::vector<Point>& line_loop);
    GeometryBuilder& exclude_from_accessible_area(const std::vector<Point>& line_loop);
    /// The walkable area as a single polygon with holes, ready to be lifted.
    PolyWithHoles build();
};
