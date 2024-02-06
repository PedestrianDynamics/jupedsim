// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AABB.hpp"

static bool intersectsLine(const AABB& boundingBox, const LineSegment& lineSegment)
{
    const Point base = lineSegment.p1;
    const Point dir = lineSegment.p2 - lineSegment.p1;
    const Point n = Point{dir.y, -dir.x};

    const Point c1 = boundingBox.BottomLeft() - base;
    const Point c2 = boundingBox.TopRight() - base;
    const Point c3 = boundingBox.BottomRight() - base;
    const Point c4 = boundingBox.TopLeft() - base;

    const double dp1 = n.ScalarProduct(c1);
    const double dp2 = n.ScalarProduct(c2);
    const double dp3 = n.ScalarProduct(c3);
    const double dp4 = n.ScalarProduct(c4);

    return (dp1 * dp2 <= 0.) || (dp2 * dp3 <= 0.) || (dp3 * dp4 <= 0.);
}

bool AABB::Intersects(const LineSegment& lineSegment) const
{
    if(!intersectsLine(*this, lineSegment)) {
        return false;
    }

    const AABB bbLineSegment({lineSegment.p1, lineSegment.p2});

    return this->Overlap(bbLineSegment);
}
