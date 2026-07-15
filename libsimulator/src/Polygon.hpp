// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <CGAL/Boolean_set_operations_2.h>

#include <ranges>
#include <tuple>
#include <vector>

class Polygon
{
    using PolygonType = Poly;

    PolygonType _polygon;

public:
    explicit Polygon(const std::vector<Point>& points);
    explicit Polygon(PolygonType polygon);
    ~Polygon() = default;
    Polygon(const Polygon& other) = default;
    Polygon& operator=(const Polygon& other) = default;
    Polygon(Polygon&& other) = default;
    Polygon& operator=(Polygon&& other) = default;
    bool IsConvex() const;
    bool IsInside(Point p) const;
    Point Centroid() const;
    std::tuple<Point, double> ContainingCircle() const;

    operator PolygonType() const { return _polygon; }
};

template <std::ranges::input_range R1, std::ranges::input_range R2>
    requires std::same_as<std::ranges::range_value_t<R1>, std::ranges::range_value_t<R2>>
PolyWithHoles CombinePolygons(R1&& polygons, R2&& exclusions)
{

    PolyWithHolesList accessibleList{};
    CGAL::join(std::begin(polygons), std::end(polygons), std::back_inserter(accessibleList));

    if(accessibleList.size() != 1) {
        throw SimulationError("Combined polygons do not form a single polygon.");
    }

    auto combinedArea = *accessibleList.begin();

    PolyWithHolesList exclusionsList{};
    CGAL::join(
        std::begin(exclusions),
        std::end(exclusions),
        std::back_inserter(exclusionsList));

    for(const auto& ex : exclusionsList) {
        PolyWithHolesList res{};
        CGAL::difference(combinedArea, ex, std::back_inserter(res));
        if(res.size() != 1) {
            throw SimulationError("Exclusions splits combined polygon.");
        }
        combinedArea = *res.begin();
    }
    return combinedArea;
}
