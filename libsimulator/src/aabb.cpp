// SPDX-License-Identifier: LGPL-3.0-or-later
#include "aabb.hpp"

#include "line_segment.hpp"
#include "point.hpp"

static bool intersects_line(const AABB& bounding_box, const LineSegment& line_segment)
{
    const Point base = line_segment.p1;
    const Point dir = line_segment.p2 - line_segment.p1;
    const Point n = Point{dir.y, -dir.x};

    const Point c1 = bounding_box.bottom_left() - base;
    const Point c2 = bounding_box.top_right() - base;
    const Point c3 = bounding_box.bottom_right() - base;
    const Point c4 = bounding_box.top_left() - base;

    const double dp1 = n.scalar_product(c1);
    const double dp2 = n.scalar_product(c2);
    const double dp3 = n.scalar_product(c3);
    const double dp4 = n.scalar_product(c4);

    return (dp1 * dp2 <= 0.) || (dp2 * dp3 <= 0.) || (dp3 * dp4 <= 0.);
}

bool AABB::intersects(const LineSegment& line_segment) const
{
    if(!intersects_line(*this, line_segment)) {
        return false;
    }

    const AABB bb_line_segment({line_segment.p1, line_segment.p2});

    return this->overlap(bb_line_segment);
}
