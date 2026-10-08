// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "GenericAgent.hpp"
#include "Point.hpp"
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
Waypoint::Waypoint(Destination destination_) : destination(destination_)
{
}

bool Waypoint::IsCompleted(const GenericAgent& agent)
{
    return destination.contains(agent.location);
}

RoutingTarget Waypoint::Target(const GenericAgent&)
{
    return destination;
}

StageProxy Waypoint::Proxy(Simulation* simulation)
{
    return WaypointProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Destination destination_, std::vector<GenericAgent::ID>& toRemove_)
    : destination(destination_), toRemove(toRemove_)
{
}

bool Exit::IsCompleted(const GenericAgent& agent)
{
    const bool hasReachedExit = destination.contains(agent.location);
    if(hasReachedExit) {
        toRemove.push_back(agent.id);
    }
    return hasReachedExit;
}

RoutingTarget Exit::Target(const GenericAgent&)
{
    return destination;
}

StageProxy Exit::Proxy(Simulation* simulation)
{
    return ExitProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// DirectSteering
////////////////////////////////////////////////////////////////////////////////
RoutingTarget DirectSteering::Target(const GenericAgent& agent)
{
    return agent.finalTarget;
}
