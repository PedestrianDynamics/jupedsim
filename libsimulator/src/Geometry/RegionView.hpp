// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/RegionSeams.hpp"
#include "Geometry/SegmentGrid.hpp"
#include "Geometry/WallMerge.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <cstddef>
#include <vector>

class Geometry3D;

/// Per-region 2D view onto the 3D surface: the walls a query in this region may find, and
/// the seams leading out of it.
///
/// A wall running past a region boundary belongs to both regions and is held by both views,
/// which is why walls carry an identity - a query looking at both has to recognise it as one
/// wall rather than repel from it twice.
class RegionView
{
public:
    using WallRange = SegmentGrid<MergedWall>::LineSegmentRange;

    RegionView(
        std::size_t regionId,
        const Geometry3D* geometry3d,
        std::vector<MergedWall> walls,
        std::vector<RegionSeam> seams);

    // Non-copyable; move-only (built once, then held by value in Geometry3D).
    RegionView(const RegionView&) = delete;
    RegionView& operator=(const RegionView&) = delete;
    RegionView(RegionView&&) = default;
    RegionView& operator=(RegionView&&) = default;

    WallRange LineSegmentsInDistanceTo(double distance, Point p) const
    {
        return _walls.LineSegmentsInDistanceTo(distance, p);
    }

    WallRange LineSegmentsInApproxDistanceTo(Point p) const
    {
        return _walls.LineSegmentsInApproxDistanceTo(p);
    }

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
    const Geometry3D* _geometry3d;
    SegmentGrid<MergedWall> _walls;
    std::vector<RegionSeam> _seams;
};
