// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "HashCombine.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <cstddef>
#include <functional>
#include <iterator>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

class Geometry2D;

double dist(LineSegment l, Point p);

/// Skips over everything further than 'distance' away from 'p' while iterating.
/// Models std::forward_iterator: it only wraps a vector iterator, so incrementing a copy leaves
/// the original untouched and a range can be traversed more than once.
template <typename T>
class DistanceQueryIterator
{
private:
    using BackingIterator = typename std::vector<T>::const_iterator;
    double _distance{};
    Point _p{};
    BackingIterator _current{};
    BackingIterator _end{};

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    /// A default constructed iterator queries nothing. It exists because std::sentinel_for
    /// requires the 'end' iterator to be semiregular, which includes default construction.
    DistanceQueryIterator() = default;

    DistanceQueryIterator(double distance, Point p, BackingIterator current, BackingIterator end)
        : _distance(distance)
        , _p(p)
        , _current(
              std::find_if(
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

    /// The dummy "int" marks this as post-increment.
    DistanceQueryIterator operator++(int)
    {
        auto before = *this;
        ++*this;
        return before;
    }

    const T& operator*() const { return *_current; }
};

/// Encodes a cell in the geometry grid.
/// Cells are defined on the intervalls [min.x, min.x + extend), [min.y, min.y + extend)
const int CELL_EXTEND = 4;
using Cell = Point;

/// Checks if two Cells are N8 neighbors. 'a' and 'b' are not considered neighbors if they have the
/// same coordinates.
bool IsN8Adjacent(const Cell& a, const Cell& b);

/// Creates a cell from a position.
/// Cells are always alligned to multiples of CELL_EXTEND. Cells are defined in worldcoordinates NOT
/// indices.
Cell makeCell(Point p);

template <>
struct std::hash<Cell> {
    std::size_t operator()(const Point& pos) const noexcept
    {
        std::hash<double> hasher{};
        return jps::hash_combine(hasher(pos.x), hasher(pos.y));
    }
};

/// Creates all cells that are trouched by the linesegment
std::set<Cell> cellsFromLineSegment(LineSegment ls);

class Geometry2D
{
private:
    PolyWithHoles _accessibleAreaPolygon;
    std::vector<LineSegment> _segments;
    std::unordered_map<Cell, std::set<LineSegment>> _grid{};
    std::unordered_map<Cell, std::vector<LineSegment>> _approximateGrid{};
    std::tuple<std::vector<Point>, std::vector<std::vector<Point>>> _accessibleArea{};

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator<LineSegment>>;
    /// Do not call constructor drectly use 'GeometryBuilder'
    /// @param segments line segments constituting the geometry
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

    LineSegmentRange LineSegmentsInApproxDistanceTo(Point p) const;

    /// Will perfrom a linesegment intersection versus the whole geometry, i.e. walls and closed
    /// doors.
    /// @param linesegment to test for intersection with geometry
    /// @return if any linesegment of the geometry was intersected.
    bool IntersectsAny(const LineSegment& linesegment) const;

    bool InsideGeometry(Point p) const;

    const std::tuple<std::vector<Point>, std::vector<std::vector<Point>>>& AccessibleArea() const;

    const PolyWithHoles& Polygon() const { return _accessibleAreaPolygon; }
    double MinApproxRadius() const { return CELL_EXTEND; }

private:
    void insertIntoApproximateGrid(const LineSegment& ls);
};
