// SPDX-License-Identifier: LGPL-3.0-or-later
#include "stage.hpp"

#include "destination.hpp"
#include "generic_agent.hpp"
#include "routing_target.hpp"

#include <cassert>
#include <vector>

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

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(
    Destination destination,
    std::vector<RemovalRecord>& to_remove,
    const SimulationClock& clock)
    : _destination(destination), _to_remove(to_remove), _clock(clock)
{
}

bool Exit::is_completed(const GenericAgent& agent)
{
    const bool has_reached_exit = _destination.contains(agent.location);
    if(has_reached_exit) {
        _to_remove.push_back({agent.id, id(), _clock.elapsed_time()});
    }
    return has_reached_exit;
}

RoutingTarget Exit::target(const GenericAgent&)
{
    return _destination;
}

////////////////////////////////////////////////////////////////////////////////
/// DirectSteering
////////////////////////////////////////////////////////////////////////////////
RoutingTarget DirectSteering::target(const GenericAgent& agent)
{
    return agent.final_target;
}
