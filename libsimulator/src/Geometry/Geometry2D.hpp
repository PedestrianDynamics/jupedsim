// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/SegmentGrid.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <tuple>
#include <vector>

class Geometry2D
{
private:
    PolyWithHoles _accessibleAreaPolygon;
    SegmentGrid _wallGrid;
    std::tuple<std::vector<Point>, std::vector<std::vector<Point>>> _accessibleArea{};

public:
    using LineSegmentRange = SegmentGrid::LineSegmentRange;
    /// Do not call constructor drectly use 'GeometryBuilder'
    /// @param accessibleArea polygon (with holes) constituting the geometry
    explicit Geometry2D(PolyWithHoles accessibleArea);
    /// Default destructor
    ~Geometry2D() = default;
    /// Copyable
    Geometry2D(const Geometry2D& other) = default;
    /// Copyable
    Geometry2D& operator=(const Geometry2D& other) = default;
    /// Movable
    Geometry2D(Geometry2D&& other) = default;
    /// Moveable
    Geometry2D& operator=(Geometry2D&& other) = default;
    /// Returns an iterator pair to all linesegments <= 'distance' away from 'p'
    /// @param distance from reference point
    /// @param p reference point
    /// @return iterator_pair to all linesegments in range
    LineSegmentRange LineSegmentsInDistanceTo(double distance, Point p) const;

    const std::vector<LineSegment>& LineSegmentsInApproxDistanceTo(Point p) const;

    /// Will perfrom a linesegment intersection versus the whole geometry, i.e. walls and closed
    /// doors.
    /// @param linesegment to test for intersection with geometry
    /// @return if any linesegment of the geometry was intersected.
    bool IntersectsAny(const LineSegment& linesegment) const;

    bool InsideGeometry(Point p) const;

    const std::tuple<std::vector<Point>, std::vector<std::vector<Point>>>& AccessibleArea() const;

    const PolyWithHoles& Polygon() const { return _accessibleAreaPolygon; }
};
