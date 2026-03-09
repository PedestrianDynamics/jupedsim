// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include "Point.hpp"

#include <tuple>
#include <vector>

std::tuple<double, double> intoTuple(const Point& p)
{
    return std::make_tuple(p.x, p.y);
}

std::vector<std::tuple<double, double>> intoTuples(const std::vector<Point>& in)
{
    std::vector<std::tuple<double, double>> tuples{};
    tuples.reserve(in.size());
    for(const auto& pt : in) {
        tuples.emplace_back(pt.x, pt.y);
    }
    return tuples;
}

Point intoPoint(const std::tuple<double, double>& p)
{
    return Point{std::get<0>(p), std::get<1>(p)};
}

std::vector<Point> intoPoints(const std::vector<std::tuple<double, double>>& in)
{
    std::vector<Point> points{};
    points.reserve(in.size());
    for(const auto& [x, y] : in) {
        points.emplace_back(x, y);
    }
    return points;
}
