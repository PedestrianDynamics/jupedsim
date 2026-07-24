// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "HashCombine.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <set>
#include <unordered_map>
#include <vector>

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

/// Spatial grid over a set of line segments (like walls or seams).
class SegmentGrid
{
private:
    std::vector<LineSegment> _segments;
    std::unordered_map<Cell, std::set<LineSegment>> _grid{};
    std::unordered_map<Cell, std::vector<LineSegment>> _approximateGrid{};

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator<LineSegment>>;
    /// @param segments line segments the grid indexes
    explicit SegmentGrid(std::vector<LineSegment> segments);
    ~SegmentGrid() = default;
    SegmentGrid(const SegmentGrid& other) = default;
    SegmentGrid& operator=(const SegmentGrid& other) = default;
    SegmentGrid(SegmentGrid&& other) = default;
    SegmentGrid& operator=(SegmentGrid&& other) = default;

    /// Returns an iterator pair to all linesegments <= 'distance' away from 'p'
    /// @param distance from reference point
    /// @param p reference point
    /// @return iterator_pair to all linesegments in range
    LineSegmentRange LineSegmentsInDistanceTo(double distance, Point p) const;

    const std::vector<LineSegment>& LineSegmentsInApproxDistanceTo(Point p) const;

    /// Performs a linesegment intersection versus every indexed segment.
    /// @param linesegment to test for intersection with the indexed segments
    /// @return if any indexed segment was intersected.
    bool IntersectsAny(const LineSegment& linesegment) const;

private:
    void insertIntoApproximateGrid(const LineSegment& ls);
};
