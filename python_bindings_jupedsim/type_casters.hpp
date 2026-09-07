// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/WalkableSurface.hpp"
#include "Point.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/typing.h>

// Type caster: According to https://pybind11.readthedocs.io/en/stable/advanced/cast/custom.html
// which coincidentally also shows a conversion for Point2D. :)
namespace pybind11::detail
{
template <>
struct type_caster<Point> {
    PYBIND11_TYPE_CASTER(Point, const_name("tuple[float, float]"));

    bool load(handle src, bool)
    {
        if(!isinstance<sequence>(src) || isinstance<str>(src)) { // Strings are sequences as well
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

template <>
struct type_caster<LineSegment> {
    PYBIND11_TYPE_CASTER(
        LineSegment,
        const_name("tuple[tuple[float, float], tuple[float, float]]"));

    bool load(handle src, bool convert)
    {
        if(!isinstance<sequence>(src) || isinstance<str>(src)) { // Strings are sequences as well
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 2) {
            return false;
        }
        auto from_caster = make_caster<Point>();
        auto to_caster = make_caster<Point>();
        if(!from_caster.load(seq[0], convert) || !to_caster.load(seq[1], convert)) {
            return false;
        }
        value = LineSegment(
            cast_op<Point&&>(std::move(from_caster)), cast_op<Point&&>(std::move(to_caster)));

        return true;
    }

    static handle cast(const LineSegment& src, return_value_policy, handle)
    {
        return make_tuple(make_tuple(src.p1.x, src.p1.y), make_tuple(src.p2.x, src.p2.y)).release();
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

namespace
{
inline std::optional<object> optional_attr(handle h, const char* name)
{
    // If attr is not found default to empty handle which carries
    // a nullptr internally.
    auto a = getattr(h, name, handle());
    if(a.ptr() == nullptr) {
        return std::nullopt;
    }
    return a;
}
} // namespace

template <>
struct type_caster<WalkableSurface::Polygon> {
    PYBIND11_TYPE_CASTER(WalkableSurface::Polygon, const_name("shapely.Polygon"));

    bool load(handle src, bool convert)
    {
        auto exterior = optional_attr(src, "exterior");
        auto interiors = optional_attr(src, "interiors");
        if(!exterior || !interiors || !isinstance<sequence>(*interiors) ||
           isinstance<str>(*interiors)) {
            return false;
        }
        auto caster = make_caster<WalkableSurface::Ring>();
        if(!caster.load((*exterior).attr("coords"), convert)) {
            return false;
        }
        value.boundary = cast_op<WalkableSurface::Ring&&>(std::move(caster));

        auto seq = reinterpret_borrow<sequence>(*interiors);
        std::vector<WalkableSurface::Ring> holes{};
        holes.reserve(seq.size());
        for(auto ring : seq) {
            auto caster = make_caster<WalkableSurface::Ring>();
            if(!caster.load(ring.attr("coords"), convert)) {
                return false;
            }
            holes.emplace_back(cast_op<WalkableSurface::Ring&&>(std::move(caster)));
        }

        value.holes = std::move(holes);
        return true;
    }
};
} // namespace pybind11::detail
