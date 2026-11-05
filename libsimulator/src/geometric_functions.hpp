// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "kernel.hpp"
#include "line_segment.hpp"

#include <CGAL/Intersections_2/Segment_2_Segment_2.h>

inline bool intersects_with_cgal(const LineSegment l1, const LineSegment l2)
{
    return CGAL::do_intersect(
        K::Segment_2(l1.p1.cgal(), l1.p2.cgal()), K::Segment_2(l2.p1.cgal(), l2.p2.cgal()));
}

inline bool intersects(const LineSegment& l1, const LineSegment& l2)
{
    return intersects_with_cgal(l1, l2);
}
