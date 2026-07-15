// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
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

// Point3D <-> tuple[float, float, float]
template <>
struct type_caster<CGAL::Exact_predicates_inexact_constructions_kernel::Point_3> {
    using Point3D = CGAL::Exact_predicates_inexact_constructions_kernel::Point_3;
    PYBIND11_TYPE_CASTER(Point3D, const_name("tuple[float, float, float]"));

    bool load(handle src, bool)
    {
        if(!isinstance<sequence>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 3) {
            return false;
        }
        value = Point3D(seq[0].cast<double>(), seq[1].cast<double>(), seq[2].cast<double>());
        return true;
    }

    static handle cast(const Point3D& src, return_value_policy, handle)
    {
        return make_tuple(src.x(), src.y(), src.z()).release();
    }
};
} // namespace pybind11::detail
