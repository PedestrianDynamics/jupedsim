// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include <algorithm>
#include <iterator>

std::tuple<double, double> intoTuple(const JPS_Point& p)
{
    return std::make_tuple(p.x, p.y);
}

std::vector<std::tuple<double, double>> intoTuple(const std::vector<JPS_Point>& p)
{
    std::vector<std::tuple<double, double>> res;
    res.reserve(p.size());
    std::transform(
        std::begin(p), std::end(p), std::back_inserter(res), [](auto&& x) { return intoTuple(x); });
    return res;
}

std::vector<std::tuple<double, double>> intoTuple(const JPS_Point* beg, const JPS_Point* end)
{
    std::vector<std::tuple<double, double>> res;
    res.reserve(end - beg);
    std::transform(beg, end, std::back_inserter(res), [](auto&& x) { return intoTuple(x); });
    return res;
}

JPS_Point intoJPS_Point(const std::tuple<double, double> p)
{
    return JPS_Point{std::get<0>(p), std::get<1>(p)};
};

std::vector<JPS_Point> intoJPS_Point(const std::vector<std::tuple<double, double>>& p)
{
    std::vector<JPS_Point> res;
    res.reserve(p.size());
    std::transform(std::begin(p), std::end(p), std::back_inserter(res), [](auto&& x) {
        return intoJPS_Point(x);
    });
    return res;
}
