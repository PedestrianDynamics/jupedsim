// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Conversion.hpp"

Point intoPoint(JPS_Point p)
{
    return {p.x, p.y};
}

JPS_Point intoJPS_Point(Point p)
{
    return {p.x, p.y};
}

JPS_Point intoJPS_Point(std::tuple<double, double> p)
{
    return {std::get<0>(p), std::get<1>(p)};
}

std::tuple<double, double> intoTuple(JPS_Point p)
{
    return std::make_tuple(p.x, p.y);
}
