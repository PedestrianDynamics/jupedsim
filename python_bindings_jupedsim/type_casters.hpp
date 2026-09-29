// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/WalkableSurface.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

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

    bool load(handle src, bool convert)
    {
        if(!isinstance<sequence>(src) || isinstance<bytes>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 2) {
            return false;
        }
        auto x_caster = make_caster<double>();
        auto y_caster = make_caster<double>();
        if(!x_caster.load(seq[0], convert) || !y_caster.load(seq[1], convert)) {
            return false;
        }
        value.x = cast_op<double&&>(std::move(x_caster));
        value.y = cast_op<double&&>(std::move(y_caster));
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
        if(!isinstance<sequence>(src)) {
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

    bool load(handle src, bool convert)
    {
        if(!isinstance<sequence>(src) || isinstance<bytes>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 3) {
            return false;
        }
        auto x_caster = make_caster<double>();
        auto y_caster = make_caster<double>();
        auto z_caster = make_caster<double>();
        if(!x_caster.load(seq[0], convert) || !y_caster.load(seq[1], convert) ||
           !z_caster.load(seq[2], convert)) {
            return false;
        }
        value = Point3D(
            cast_op<double&&>(std::move(x_caster)),
            cast_op<double&&>(std::move(y_caster)),
            cast_op<double&&>(std::move(z_caster)));
        return true;
    }

    static handle cast(const Point3D& src, return_value_policy, handle)
    {
        return make_tuple(src.x(), src.y(), src.z()).release();
    }
};

// jps::UniqueID<Tag, Integer> -> int, C++ -> Python only.
//
// Deliberately NOT using PYBIND11_TYPE_CASTER: the macro declares a default-initialized
// `value` member, and UniqueID's default constructor draws a fresh id from the global
// counter. Every caster instantiation would then silently consume an id. The members the
// macro would generate are spelled out below with `value` initialised to ID::Invalid.
template <typename Tag, typename Integer>
struct type_caster<jps::UniqueID<Tag, Integer>> {
    using ID = jps::UniqueID<Tag, Integer>;

    static constexpr auto name = const_name("int");

    // Python -> C++ is intentionally unsupported.
    bool load(handle, bool) { return false; }

    static handle cast(const ID& src, return_value_policy policy, handle parent)
    {
        return make_caster<Integer>::cast(src.getID(), policy, parent);
    }

    // Required by pybind11's caster protocol even though load() never succeeds.
    template <typename T>
    using cast_op_type = pybind11::detail::cast_op_type<T>;
    operator ID&() { return value; }
    operator ID*() { return &value; }

protected:
    ID value{ID::Invalid};
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
        if(!exterior || !interiors || !isinstance<sequence>(*interiors)) {
            return false;
        }
        auto boundary_coords = optional_attr(*exterior, "coords");
        if(!boundary_coords) {
            return false;
        }
        auto caster = make_caster<WalkableSurface::Ring>();
        if(!caster.load(*boundary_coords, convert)) {
            return false;
        }
        value.boundary = cast_op<WalkableSurface::Ring&&>(std::move(caster));

        auto seq = reinterpret_borrow<sequence>(*interiors);
        std::vector<WalkableSurface::Ring> holes{};
        holes.reserve(seq.size());
        for(auto ring : seq) {
            auto hole_coords = optional_attr(ring, "coords");
            if(!hole_coords) {
                return false;
            }
            auto caster = make_caster<WalkableSurface::Ring>();
            if(!caster.load(*hole_coords, convert)) {
                return false;
            }
            holes.emplace_back(cast_op<WalkableSurface::Ring&&>(std::move(caster)));
        }

        value.holes = std::move(holes);
        return true;
    }
};
} // namespace pybind11::detail
