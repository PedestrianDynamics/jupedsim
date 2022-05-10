#pragma once

#include "IteratorPair.hpp"
#include "Line.hpp"

#include <vector>

class Geometry;

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

class Geometry
{
    std::vector<Line> _segments;

public:
    using LineSegmentRange = IteratorPair<DistanceQueryIterator<Line>>;
    /// Do not call constructor drectly use 'GeometryBuilder'
    /// @param segments line segments constituting the geometry
    explicit Geometry(std::vector<Line>&& segments);
    /// Default destructor
    ~Geometry() = default;
    /// Non-copyable
    Geometry(const Geometry& other) = delete;
    /// Non-copyable
    Geometry& operator=(const Geometry& other) = delete;
    /// Non-movable
    Geometry(Geometry&& other) = default;
    /// Non-moveable
    Geometry& operator=(Geometry&& other) = default;
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

class GeometryBuilder
{
    std::vector<Line> _segements;

public:
    /// Default constructor
    GeometryBuilder() = default;
    /// Default destructor
    ~GeometryBuilder() = default;
    /// Non-copyable
    GeometryBuilder(const GeometryBuilder& other) = delete;
    /// Non-copyable
    GeometryBuilder& operator=(const GeometryBuilder& other) = delete;
    /// Non-movable
    GeometryBuilder(GeometryBuilder&& other) = delete;
    /// Non-movable
    GeometryBuilder& operator=(GeometryBuilder&& other) = delete;
    /// Add linesegment to static geometry
    /// @param x1 x cordinate of first point of line segment
    /// @param y1 y cordinate of first point of line segment
    /// @param x2 x cordinate of second point of line segment
    /// @param y2 y cordinate of second point of line segment
    /// @return GeometryBuilder to chaining calls
    GeometryBuilder& AddLineSegment(double x1, double y1, double x2, double y2);
    /// Add door to geometry
    /// @param x1 x cordinate of first point of line segment
    /// @param y1 y cordinate of first point of line segment
    /// @param x2 x cordinate of second point of line segment
    /// @param y2 y cordinate of second point of line segment
    /// @param id of the door for later maniputation (open/close)
    /// @return GeometryBuilder to chaining calls
    GeometryBuilder& AddDoor(double x1, double y1, double x2, double y2, int id);
    /// Finishes Geometry construction and creates 'Geometry' from the builder.
    /// @return Geometry with all added line sements.
    Geometry Build();
};
