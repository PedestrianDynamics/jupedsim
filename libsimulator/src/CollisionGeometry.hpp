#pragma once

#include "IteratorPair.hpp"
#include "Line.hpp"

#include <vector>

class CollisionGeometry;

double dist(Line l, Point p);

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
    std::vector<Line> _segments;

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator<Line>>;
    /// Do not call constructor drectly use 'GeometryBuilder'
    /// @param segments line segments constituting the geometry
    explicit CollisionGeometry(std::vector<Line>&& segments);
    /// Default destructor
    ~CollisionGeometry() = default;
    /// Non-copyable
    CollisionGeometry(const CollisionGeometry& other) = delete;
    /// Non-copyable
    CollisionGeometry& operator=(const CollisionGeometry& other) = delete;
    /// Non-movable
    CollisionGeometry(CollisionGeometry&& other) = default;
    /// Non-moveable
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
    bool IntersectsAny(Line linesegment) const;

    /// The following methods are temporay until we have completely migrated from building to
    /// geometry and function to support the required geometry modification for trains in the mean
    /// time.
    void AddLineSegment(Line l);
    void RemoveLineSegment(Line l);
};

class CollisionGeometryBuilder
{
    std::vector<Line> _segements;

public:
    /// Default constructor
    CollisionGeometryBuilder() = default;
    /// Default destructor
    ~CollisionGeometryBuilder() = default;
    /// Non-copyable
    CollisionGeometryBuilder(const CollisionGeometryBuilder& other) = delete;
    /// Non-copyable
    CollisionGeometryBuilder& operator=(const CollisionGeometryBuilder& other) = delete;
    /// Non-movable
    CollisionGeometryBuilder(CollisionGeometryBuilder&& other) = delete;
    /// Non-movable
    CollisionGeometryBuilder& operator=(CollisionGeometryBuilder&& other) = delete;
    /// Add linesegment to static geometry
    /// @param x1 x cordinate of first point of line segment
    /// @param y1 y cordinate of first point of line segment
    /// @param x2 x cordinate of second point of line segment
    /// @param y2 y cordinate of second point of line segment
    /// @return GeometryBuilder to chaining calls
    CollisionGeometryBuilder& AddLineSegment(double x1, double y1, double x2, double y2);
    /// Add door to geometry
    /// @param x1 x cordinate of first point of line segment
    /// @param y1 y cordinate of first point of line segment
    /// @param x2 x cordinate of second point of line segment
    /// @param y2 y cordinate of second point of line segment
    /// @param id of the door for later maniputation (open/close)
    /// @return GeometryBuilder to chaining calls
    CollisionGeometryBuilder& AddDoor(double x1, double y1, double x2, double y2, int id);
    /// Finishes Geometry construction and creates 'Geometry' from the builder.
    /// @return Geometry with all added line sements.
    CollisionGeometry Build();
};
