// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/SegmentGrid.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <cstddef>
#include <vector>

class Geometry3D;

/// Merge all collinear segments using Douglas-Peucker algorithm. @p eps defines the max
// perpendicular distance used.
std::vector<LineSegment> merge_collinear(const std::vector<LineSegment>& segments, double eps);

/// Per-region 2D view onto the 3D surface. Holds this region's wall segments
/// and separately its seam segments.
class RegionView
{
public:
    using LineSegmentRange = SegmentGrid::LineSegmentRange;

    RegionView(
        std::size_t regionId,
        const Geometry3D* geometry3d,
        std::vector<LineSegment> walls,
        std::vector<LineSegment> seams,
        std::vector<std::size_t> seamNeighbors);

    // Non-copyable; move-only (built once, then held by value in Geometry3D).
    RegionView(const RegionView&) = delete;
    RegionView& operator=(const RegionView&) = delete;
    RegionView(RegionView&&) = default;
    RegionView& operator=(RegionView&&) = default;

    LineSegmentRange LineSegmentsInDistanceTo(double distance, Point p) const;
    const std::vector<LineSegment>& LineSegmentsInApproxDistanceTo(Point p) const;
    bool IntersectsAny(const LineSegment& linesegment) const;

    /// Checks whether 2D coordinates are inside this region
    bool InsideGeometry(Point p) const;

    /// Seam-adjacent regions (z-continuous neighbours), ascending.
    const std::vector<std::size_t>& seam_neighbors() const { return _seamNeighbors; }

    /// True iff a-b crosses one of this region's seam segments.
    bool crosses_seam(Point a, Point b) const;

private:
    std::size_t _regionId;
    const Geometry3D* _geometry3d;
    SegmentGrid _walls;
    SegmentGrid _seams;
    std::vector<std::size_t> _seamNeighbors;
};
