#include "Journey.hpp"
#include <memory>
#include <tuple>
////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////

void SimpleJourney::AddWaypoint(Point p, double distance)
{
    waypoints.emplace_back(std::make_tuple(p, distance));
}

std::unique_ptr<Journey> SimpleJourney::Clone() const
{
    return std::make_unique<SimpleJourney>(*this);
}
