// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <vector>

/// Wall queries on 3D surface.
class WallIndex
{
    std::vector<Segment3D> _edges{};

public:
    explicit WallIndex(const SurfaceMesh& mesh);

    const std::vector<Segment3D>& segments() const { return _edges; }

    /// "Wall" edges within horizontal distance `radius` of `p` whose height at
    /// the (2D-)closest footpoint differs compared to p.z by less than `height`.
    std::vector<Segment3D> segments_near(const Point3D& p, double radius, double height) const;

    /// True iff the 2D projection of a->b crosses no "wall" edge whose
    /// interpolated height at the crossing differs from the height of a->b at
    /// that point by less than `height`.
    bool is_visible(const Point3D& a, const Point3D& b, double height) const;

    /// ranges-filter (--> returns filter function):
    ///  `other` is visible from `p` (no blocking wall in between).
    auto visible_from(const Point3D& p, double height) const
    {
        return [this, p, height](const Point3D& other) { return is_visible(p, other, height); };
    }
};
