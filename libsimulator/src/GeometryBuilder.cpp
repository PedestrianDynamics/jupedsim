// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CfgCgal.hpp"
#include "Point.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <vector>

GeometryBuilder& GeometryBuilder::add_accessible_area(const std::vector<Point>& line_loop)
{
    _accessible_areas.emplace_back(line_loop);
    return *this;
}

GeometryBuilder& GeometryBuilder::exclude_from_accessible_area(const std::vector<Point>& line_loop)
{
    _exclusions.emplace_back(line_loop);
    return *this;
}

PolyWithHoles GeometryBuilder::build()
{
    const std::vector<Poly> accessible_areas{
        std::begin(_accessible_areas), std::end(_accessible_areas)};
    const std::vector<Poly> exclusions{std::begin(_exclusions), std::end(_exclusions)};
    return combine_polygons(accessible_areas, exclusions);
}
