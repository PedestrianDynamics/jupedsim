// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CfgCgal.hpp"
#include "Geometry/Geometry2D.hpp"
#include "Point.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <vector>

GeometryBuilder& GeometryBuilder::AddAccessibleArea(const std::vector<Point>& lineLoop)
{
    _accessibleAreas.emplace_back(lineLoop);
    return *this;
}

GeometryBuilder& GeometryBuilder::ExcludeFromAccessibleArea(const std::vector<Point>& lineLoop)
{
    _exclusions.emplace_back(lineLoop);
    return *this;
}

Geometry2D GeometryBuilder::Build()
{
    const std::vector<Poly> accessibleAreas{
        std::begin(_accessibleAreas), std::end(_accessibleAreas)};
    const std::vector<Poly> exclusions{std::begin(_exclusions), std::end(_exclusions)};
    return Geometry2D(CombinePolygons(accessibleAreas, exclusions));
}
