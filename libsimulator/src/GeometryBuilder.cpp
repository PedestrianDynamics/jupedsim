// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeometryBuilder.hpp"

#include "CfgCgal.hpp"
#include "CollisionGeometry.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <memory>
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

CollisionGeometry GeometryBuilder::Build()
{
    const std::vector<Poly> accessibleListInput{
        std::begin(_accessibleAreas), std::end(_accessibleAreas)};
    PolyWithHolesList accessibleList{};

    CGAL::join(
        std::begin(accessibleListInput),
        std::end(accessibleListInput),
        std::back_inserter(accessibleList));

    if(accessibleList.size() != 1) {
        throw SimulationError("accessible area not connected");
    }

    auto accessibleArea = *accessibleList.begin();

    const std::vector<Poly> exclusionsListInput{std::begin(_exclusions), std::end(_exclusions)};
    PolyWithHolesList exclusionsList{};
    CGAL::join(
        std::begin(exclusionsListInput),
        std::end(exclusionsListInput),
        std::back_inserter(exclusionsList));

    for(const auto& ex : exclusionsList) {
        PolyWithHolesList res{};
        CGAL::difference(accessibleArea, ex, std::back_inserter(res));
        if(res.size() != 1) {
            throw SimulationError("Exclusion splits accessibleArea");
        }
        accessibleArea = *res.begin();
    }

    return CollisionGeometry(accessibleArea);
}
