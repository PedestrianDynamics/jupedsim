// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <Point.hpp>

#include <tuple>
#include <vector>

std::tuple<double, double> intoTuple(const Point& p);

// Works for container of Point type, but also e.g. for container of glm::vec<2>
std::vector<std::tuple<double, double>> intoTuples(const auto& in)
{
    std::vector<std::tuple<double, double>> tuples{};
    tuples.reserve(in.size());
    for(const auto& pt : in) {
        tuples.emplace_back(pt.x, pt.y);
    }
    return tuples;
}


Point intoPoint(const std::tuple<double, double>& p);

std::vector<Point> intoPoints(const std::vector<std::tuple<double, double>>& in);

template <typename T, typename U>
std::vector<T> intoVecT(const std::vector<U>& vec)
{
    auto result = std::vector<T>();
    result.reserve(vec.size());
    for(const auto& v : vec) {
        result.emplace_back(v);
    }
    return result;
}
