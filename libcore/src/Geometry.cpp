#include "Geometry.hpp"

#include "IteratorPair.hpp"

#include <algorithm>
#include <vector>

double dist(Line l, Point p)
{
    return l.DistTo(p);
}

double dist(Door d, Point p)
{
    return dist(d.linesegment, p);
}

Geometry::Geometry(std::vector<Line>&& segments, std::vector<Door>&& doors)
    : _segments(std::move(segments)), _doors(std::move(doors))
{
}

Geometry::LineSegmentRange Geometry::LineSegmentsInDistanceTo(double distance, Point p) const
{
    return LineSegmentRange{
        DistanceQueryIterator<Line>{distance, p, _segments.cbegin(), _segments.cend()},
        DistanceQueryIterator<Line>{distance, p, _segments.cend(), _segments.cend()}};
}

Geometry::DoorRange Geometry::DoorsInDistanceTo(double distance, Point p) const
{
    return Geometry::DoorRange{
        DistanceQueryIterator<Door>{distance, p, _doors.cbegin(), _doors.cend()},
        DistanceQueryIterator<Door>{distance, p, _doors.cend(), _doors.cend()}};
}

bool Geometry::IntersectsAny(Line linesegment) const
{
    const bool intersects_wall =
        std::find_if(_segments.cbegin(), _segments.cend(), [&linesegment](const auto& segment) {
            return linesegment.IntersectionWith(segment);
        }) != _segments.end();
    if(intersects_wall) {
        return true;
    }

    const bool insersects_closed_door =
        std::find_if(_doors.begin(), _doors.end(), [&linesegment](const auto& d) {
            return d.state != DoorState::OPEN && d.linesegment.IntersectionWith(linesegment);
        }) != _doors.end();
    return insersects_closed_door;
}

void Geometry::UpdateDoorState(int id, DoorState newState)
{
    if(const auto iter =
           std::find_if(_doors.begin(), _doors.end(), [id](const auto& d) { return d.id == id; });
       iter != _doors.end()) {
        iter->state = newState;
    }
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

GeometryBuilder& GeometryBuilder::AddDoor(double x1, double y1, double x2, double y2, int id)
{
    _doors.push_back(Door{Line(Point(x1, y1), Point(x2, y2)), id, DoorState::OPEN});
    return *this;
}

Geometry GeometryBuilder::Build()
{
    _segements.shrink_to_fit();
    _doors.shrink_to_fit();
    return Geometry{std::move(_segements), std::move(_doors)};
}
