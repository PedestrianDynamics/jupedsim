/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "IteratorPair.hpp"
#include "LineSegment.hpp"

#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using PolyWithHoles = CGAL::Polygon_with_holes_2<Kernel>;
using Poly = CGAL::Polygon_2<Kernel>;

class CollisionGeometry;

double dist(LineSegment l, Point p);

template <typename T>
class DistanceQueryIterator
{
private:
    using BackingIterator = typename std::vector<T>::const_iterator;
    double _distance;
    Point _p;
    BackingIterator _current;
    BackingIterator _end;

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    DistanceQueryIterator(double distance, Point p, BackingIterator current, BackingIterator end)
        : _distance(distance)
        , _p(p)
        , _current(std::find_if(
              current,
              end,
              [this](const auto& t) { return dist(t, _p) <= _distance; }))
        , _end(end)
    {
    }
    ~DistanceQueryIterator() = default;
    DistanceQueryIterator(const DistanceQueryIterator& other) = default;
    DistanceQueryIterator& operator=(const DistanceQueryIterator& other) = default;

    bool operator==(const DistanceQueryIterator& other) const { return _current == other._current; }

    bool operator!=(const DistanceQueryIterator& other) const { return !(*this == other); }

    DistanceQueryIterator& operator++()
    {
        do {
            ++_current;
        } while(_current != _end && dist(*_current, _p) > _distance);
        return *this;
    }

    const T& operator*() const { return *_current; }
};

class CollisionGeometry
{
    PolyWithHoles _accessibleArea;
    std::vector<LineSegment> _segments;

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator<LineSegment>>;
    /// Do not call constructor drectly use 'GeometryBuilder'
    /// @param segments line segments constituting the geometry
    explicit CollisionGeometry(PolyWithHoles accessibleArea);
    /// Default destructor
    ~CollisionGeometry() = default;
    /// Copyable
    CollisionGeometry(const CollisionGeometry& other) = default;
    /// Copyable
    CollisionGeometry& operator=(const CollisionGeometry& other) = default;
    /// Movable
    CollisionGeometry(CollisionGeometry&& other) = default;
    /// Moveable
    CollisionGeometry& operator=(CollisionGeometry&& other) = default;
    /// Returns an iterator pair to all linesegments <= 'distance' away from 'p'
    /// @param distance from reference point
    /// @param p reference point
    /// @return iterator_pair to all linesegments in range
    LineSegmentRange LineSegmentsInDistanceTo(double distance, Point p) const;
    /// Will perfrom a linesegment intersection versus the whole geometry, i.e. walls and closed
    /// doors.
    /// @param linesegment to test for intersection with geometry
    /// @return if any linesegment of the geometry was intersected.
    bool IntersectsAny(LineSegment linesegment) const;

    bool InsideGeometry(Point p) const;
};
