// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <Point.hpp>

#include <tuple>
#include <vector>

std::tuple<double, double> intoTuple(const Point& p);

std::vector<std::tuple<double, double>> intoTuples(const std::vector<Point>& in);

Point intoPoint(const std::tuple<double, double>& p);

std::vector<Point> intoPoints(const std::vector<std::tuple<double, double>>& in);

template <typename T, typename U>
std::vector<T> intoVecT(const std::vector<U>& vec)
{
    auto result = std::vector<T>();
    result.reserve(vec.size());
    for(const auto& v : vec) {
        result.push_back(v);
    }
    return result;
}