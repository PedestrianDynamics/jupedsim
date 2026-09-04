// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AABB.hpp"
#include "GeometricFunctions.hpp"
#include "HashCombine.hpp"
#include "IteratorPair.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <set>
#include <unordered_map>
#include <vector>

/// Skips over everything further than 'distance' away from 'p' while iterating.
/// Models std::forward_iterator: it only wraps a vector iterator, so incrementing a copy leaves
/// the original untouched and a range can be traversed more than once.
class DistanceQueryIterator
{
private:
    using BackingIterator = std::vector<LineSegment>::const_iterator;
    double _distance{};
    Point _p{};
    BackingIterator _current{};
    BackingIterator _end{};

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = LineSegment;
    using difference_type = std::ptrdiff_t;
    using pointer = const LineSegment*;
    using reference = const LineSegment&;

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
                  [this](const LineSegment& ls) { return ls.DistTo(_p) <= _distance; }))
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
        } while(_current != _end && _current->DistTo(_p) > _distance);
        return *this;
    }

    /// The dummy "int" marks this as post-increment.
    DistanceQueryIterator operator++(int)
    {
        auto before = *this;
        ++*this;
        return before;
    }

    const LineSegment& operator*() const { return *_current; }
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

/// Cell membership and the search radius are computed from the inflated bounds of a segment.
inline AABB search_bounds(const LineSegment& ls, double radius)
{
    const AABB bounds({ls.p1, ls.p2});
    const Point extend(radius, radius);
    return AABB(bounds.BottomLeft() - extend, bounds.TopRight() + extend);
}

/// Spatial grid over a set of line segments (like walls or seams).
class SegmentGrid
{
private:
    std::vector<LineSegment> _segments;
    std::unordered_map<Cell, std::set<LineSegment>> _grid{};
    std::unordered_map<Cell, std::vector<LineSegment>> _approximateGrid{};

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator>;

    /// @param segments line segments the grid indexes
    explicit SegmentGrid(std::vector<LineSegment> segments) : _segments(std::move(segments))
    {
        for(const auto& segment : _segments) {
            for(const auto& cell : cellsFromLineSegment(segment)) {
                _grid[cell].insert(segment);
            }
            insertIntoApproximateGrid(segment);
        }
        for(auto& [_, vec] : _approximateGrid) {
            vec.shrink_to_fit();
        }
    }

    /// Returns an iterator pair to all linesegments <= 'distance' away from 'p'
    /// @param distance from reference point
    /// @param p reference point
    /// @return iterator_pair to all linesegments in range
    LineSegmentRange LineSegmentsInDistanceTo(double distance, Point p) const
    {
        return LineSegmentRange{
            DistanceQueryIterator{distance, p, _segments.cbegin(), _segments.cend()},
            DistanceQueryIterator{distance, p, _segments.cend(), _segments.cend()}};
    }

    LineSegmentRange LineSegmentsInApproxDistanceTo(Point p) const
    {
        constexpr double inf = std::numeric_limits<double>::infinity();
        static const std::vector<LineSegment> empty{};
        const auto it = _approximateGrid.find(makeCell(p));
        const auto& vec = (it != _approximateGrid.end()) ? it->second : empty;
        return {
            DistanceQueryIterator(inf, p, vec.begin(), vec.end()),
            DistanceQueryIterator(inf, p, vec.end(), vec.end())};
    }

    /// Smallest radius for which the approximate query is meaningful (the grid cell size).
    double MinApproxRadius() const { return CELL_EXTEND; }

    /// Performs a linesegment intersection versus every indexed segment.
    /// @param linesegment to test for intersection with the indexed segments
    /// @return if any indexed segment was intersected.
    bool IntersectsAny(const LineSegment& linesegment) const
    {
        for(const auto& cell : cellsFromLineSegment(linesegment)) {
            const auto iter = _grid.find(cell);
            if(iter == std::end(_grid)) {
                continue;
            }
            if(std::any_of(
                   iter->second.cbegin(),
                   iter->second.cend(),
                   [&linesegment](const auto& candidate) {
                       return intersects(linesegment, candidate);
                   })) {
                return true;
            }
        }
        return false;
    }

private:
    void insertIntoApproximateGrid(const LineSegment& segment)
    {
        constexpr double searchRadius = 4.;
        const AABB bounds = search_bounds(segment, searchRadius);
        const auto bottomLeft = makeCell(bounds.BottomLeft());
        const auto topRight = makeCell(bounds.TopRight());

        for(double x = bottomLeft.x; x <= topRight.x; x += CELL_EXTEND) {
            for(double y = bottomLeft.y; y <= topRight.y; y += CELL_EXTEND) {
                const auto cell = makeCell({x, y});
                const AABB cellWithSearchRadius(
                    {cell.x - searchRadius, cell.y - searchRadius},
                    {cell.x + searchRadius + CELL_EXTEND, cell.y + searchRadius + CELL_EXTEND});
                if(cellWithSearchRadius.Intersects(segment)) {
                    _approximateGrid[cell].push_back(segment);
                }
            }
        }
    }
};
