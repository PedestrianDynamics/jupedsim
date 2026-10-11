// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "point.hpp"
#include "simulation_error.hpp"

#include <CGAL/Boolean_set_operations_2.h>

#include <ranges>
#include <string>
#include <tuple>
#include <vector>

class Polygon
{
    using PolygonType = Poly;

    PolygonType _polygon;

public:
    explicit Polygon(const std::vector<Point>& points);
    explicit Polygon(PolygonType polygon);
    /// Approximate polygon with corners on the circle
    static Polygon from_circle(Point center, double radius);
    ~Polygon() = default;
    Polygon(const Polygon& other) = default;
    Polygon& operator=(const Polygon& other) = default;
    Polygon(Polygon&& other) = default;
    Polygon& operator=(Polygon&& other) = default;
    bool is_convex() const;
    bool is_inside(Point p) const;
    Point centroid() const;
    std::tuple<Point, double> containing_circle() const;

    operator PolygonType() const { return _polygon; }
};

template <std::ranges::input_range R1, std::ranges::input_range R2>
    requires std::same_as<std::ranges::range_value_t<R1>, std::ranges::range_value_t<R2>>
PolyWithHoles combine_polygons(R1&& polygons, R2&& exclusions)
{

    PolyWithHolesList accessible_list{};
    CGAL::join(std::begin(polygons), std::end(polygons), std::back_inserter(accessible_list));

    if(accessible_list.size() != 1) {
        throw SimulationError("Combined polygons do not form a single polygon.");
    }

    auto combined_area = *accessible_list.begin();

    PolyWithHolesList exclusions_list{};
    CGAL::join(std::begin(exclusions), std::end(exclusions), std::back_inserter(exclusions_list));

    for(const auto& ex : exclusions_list) {
        PolyWithHolesList res{};
        CGAL::difference(combined_area, ex, std::back_inserter(res));
        if(res.size() != 1) {
            throw SimulationError("Exclusions splits combined polygon.");
        }
        combined_area = *res.begin();
    }
    return combined_area;
}

/// Throws if @p polygon is empty.
std::string as_wkt(const PolyWithHoles& polygon);
