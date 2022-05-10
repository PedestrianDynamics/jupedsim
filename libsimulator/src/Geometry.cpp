#include "Geometry.hpp"

#include "IteratorPair.hpp"

#include <algorithm>
#include <vector>

double dist(Line l, Point p)
{
    return l.DistTo(p);
}

Geometry::Geometry(std::vector<Line>&& segments) : _segments(std::move(segments))
{
}

Geometry::LineSegmentRange Geometry::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return LineSegmentRange{
        DistanceQueryIterator<Line>{distance, p, _segments.cbegin(), _segments.cend()},
        DistanceQueryIterator<Line>{distance, p, _segments.cend(), _segments.cend()}};
}

bool Geometry::IntersectsAny(Line linesegment) const
{
    return std::find_if(_segments.cbegin(), _segments.cend(), [&linesegment](const auto& segment) {
               return linesegment.IntersectionWith(segment);
           }) != _segments.end();
}

void Geometry::AddLineSegment(Line l)
{
    _segments.push_back(l);
}

void Geometry::RemoveLineSegment(Line l)
{
    _segments.erase(std::remove(_segments.begin(), _segments.end(), l), _segments.end());
}

GeometryBuilder& GeometryBuilder::AddLineSegment(double x1, double y1, double x2, double y2)
{
    _segements.emplace_back(Point{x1, y1}, Point{x2, y2});
    return *this;
}

Geometry GeometryBuilder::Build()
{
    _segements.shrink_to_fit();
    return Geometry{std::move(_segements)};
}
