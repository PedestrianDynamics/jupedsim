// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/RegionSeams.hpp"
#include "Geometry/SegmentGrid.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <cstddef>
#include <vector>

class Geometry;

/// Per-region 2D view onto the 3D surface: this region's own boundary, and the seams leading
/// out of it. What a query gets to see is `BoundaryIndex`'s business; this is what answers
/// whether something is in the way inside the region.
class RegionView
{
public:
    RegionView(
        std::size_t regionId,
        const Geometry* geometry3d,
        SegmentGrid walls,
        std::vector<RegionSeam> seams);

    // Non-copyable; move-only (built once, then held by value in Geometry).
    RegionView(const RegionView&) = delete;
    RegionView& operator=(const RegionView&) = delete;
    RegionView(RegionView&&) = default;
    RegionView& operator=(RegionView&&) = default;

    bool IntersectsAny(const LineSegment& linesegment) const
    {
        return _walls.IntersectsAny(linesegment);
    }

    /// Checks whether 2D coordinates are inside this region
    bool InsideGeometry(Point p) const;

    /// The seams leading out of this region, each naming what lies across it.
    const std::vector<RegionSeam>& seams() const { return _seams; }

    /// True iff a-b crosses one of this region's seam segments.
    bool crosses_seam(Point a, Point b) const;

private:
    std::size_t _regionId;
    const Geometry* _geometry3d;
    SegmentGrid _walls;
    std::vector<RegionSeam> _seams;
};
