// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AABB.hpp"
#include "LineSegment.hpp"

#include <algorithm>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>
#include <boost/variant.hpp>

/// Computes the cross product 2D of vectors apex->a (A) and apex->b (B)
/// Geometrically the absolute value of the cross product in 2d is also the area of the
/// parallelogram of A and B. The sign of the cross product determines the relation of A and B.
/// If AxB < 0 A is left of B
/// if AxB > 0 A is right of B
/// otherwise A and B are parallel
template <typename PointType>
inline auto triarea2d(PointType apex, PointType a, PointType b)
{
    const float ax = a.x - apex.x;
    const float ay = a.y - apex.y;
    const float bx = b.x - apex.x;
    const float by = b.y - apex.y;
    return bx * ay - ax * by;
}

enum class Orientation { Colinear, Clockwise, CounterClockwise };

template <typename PointType>
inline auto orientation(PointType a, PointType b, PointType c)
{
    const auto slope_ab = (b.y - a.y) / (b.x - a.x);
    const auto slope_bc = (c.y - b.y) / (c.x - b.x);
    const auto slope_delta = slope_ab - slope_bc;

    if(slope_delta > 0) {
        return Orientation::Clockwise;
    } else if(slope_delta < 0) {
        return Orientation::CounterClockwise;
    }
    return Orientation::Colinear;
}

template <typename PointType>
inline bool intersects(PointType a1, PointType a2, PointType b1, PointType b2)
{
    const auto o1 = orientation(a1, a2, b1);
    const auto o2 = orientation(a1, a2, b2);
    const auto o3 = orientation(b1, b2, a1);
    const auto o4 = orientation(b1, b2, a2);
    if(o1 != o2 && o3 != o4) {
        return true;
    }
    const auto xy_projection = [](const auto p1, const auto p2, const auto c) {
        const AABB aabb(p1, p2);
        return aabb.Inside(c);
    };
    if(o1 == Orientation::Colinear && xy_projection(a1, a2, b1)) {
        return true;
    }
    if(o2 == Orientation::Colinear && xy_projection(a1, a2, b2)) {
        return true;
    }
    if(o3 == Orientation::Colinear && xy_projection(b1, b2, a1)) {
        return true;
    }
    if(o4 == Orientation::Colinear && xy_projection(b1, b2, a2)) {
        return true;
    }
    return false;
}

inline bool intersectsWithCGAL(const LineSegment l1, const LineSegment l2)
{
    using K = CGAL::Exact_predicates_inexact_constructions_kernel;
    using Point_2 = K::Point_2;
    using Segment_2 = K::Segment_2;
    const Segment_2 this_segment(Point_2(l1.p1.x, l1.p1.y), Point_2(l1.p2.x, l1.p2.y));
    const Segment_2 other_segment(Point_2(l2.p1.x, l2.p1.y), Point_2(l2.p2.x, l2.p2.y));
    return CGAL::do_intersect(this_segment, other_segment);
}

inline bool intersects(const LineSegment& l1, const LineSegment& l2)
{
    return intersectsWithCGAL(l1, l2);
}
