// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "GenericAgent.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
/// Base Proxy
////////////////////////////////////////////////////////////////////////////////
size_t BaseProxy::CountTargeting() const
{
    return stage->CountTargeting();
}

////////////////////////////////////////////////////////////////////////////////
/// Waypoint
////////////////////////////////////////////////////////////////////////////////
Waypoint::Waypoint(Location position_, double distance_) : position(position_), distance(distance_)
{
}

bool Waypoint::IsCompleted(const GenericAgent& agent)
{
    return agent.location.distance_to(position) <= distance;
}

Location Waypoint::Target(const GenericAgent&)
{
    return position;
}

StageProxy Waypoint::Proxy(Simulation* simulation)
{
    return WaypointProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Polygon area_, Location centroid_, std::vector<GenericAgent::ID>& toRemove_)
    : area(std::move(area_)), centroid(centroid_), toRemove(toRemove_)
{
    if(!area.IsConvex()) {
        throw SimulationError("Exit areas need to be bounded by convex polygons.");
    }
}

bool Exit::IsCompleted(const GenericAgent& agent)
{
    const bool hasReachedExit =
        area.IsInside(agent.location.xy()) && agent.location.can_walk_straight_to(centroid);
    if(hasReachedExit) {
        toRemove.push_back(agent.id);
    }
    return hasReachedExit;
}

Location Exit::Target(const GenericAgent&)
{
    return centroid;
}

StageProxy Exit::Proxy(Simulation* simulation)
{
    return ExitProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// DirectSteering
////////////////////////////////////////////////////////////////////////////////
Location DirectSteering::Target(const GenericAgent& agent)
{
    return agent.finalTarget;
}
