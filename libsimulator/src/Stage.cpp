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
size_t BaseProxy::count_targeting() const
{
    return _stage->count_targeting();
}

////////////////////////////////////////////////////////////////////////////////
/// Waypoint
////////////////////////////////////////////////////////////////////////////////
Waypoint::Waypoint(Destination destination) : _destination(destination)
{
}

bool Waypoint::is_completed(const GenericAgent& agent)
{
    return _destination.contains(agent.location);
}

RoutingTarget Waypoint::target(const GenericAgent&)
{
    return _destination;
}

StageProxy Waypoint::proxy(Simulation* simulation)
{
    return WaypointProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Destination destination, std::vector<GenericAgent::ID>& to_remove)
    : _destination(destination), _to_remove(to_remove)
{
}

bool Exit::is_completed(const GenericAgent& agent)
{
    const bool has_reached_exit = _destination.contains(agent.location);
    if(has_reached_exit) {
        _to_remove.push_back(agent.id);
    }
    return has_reached_exit;
}

RoutingTarget Exit::target(const GenericAgent&)
{
    return _destination;
}

StageProxy Exit::proxy(Simulation* simulation)
{
    return ExitProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// DirectSteering
////////////////////////////////////////////////////////////////////////////////
RoutingTarget DirectSteering::target(const GenericAgent& agent)
{
    return agent.final_target;
}
