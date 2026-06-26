// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <pybind11/pybind11.h>

// Type caster: According to https://pybind11.readthedocs.io/en/stable/advanced/cast/custom.html
// which coincidentally also shows a conversion for Point2D. :)
namespace pybind11::detail
{
template <>
struct type_caster<Point> {
    PYBIND11_TYPE_CASTER(Point, const_name("tuple[float, float]"));

    bool load(handle src, bool)
    {
        if(!isinstance<sequence>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 2) {
            return false;
        }
        value.x = seq[0].cast<double>();
        value.y = seq[1].cast<double>();
        return true;
    }

    static handle cast(const Point& src, return_value_policy, handle)
    {
        return make_tuple(src.x, src.y).release();
    }
};
} // namespace pybind11::detail
