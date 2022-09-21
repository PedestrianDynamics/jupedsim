/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "TacticalDecisionSystem.hpp"

#include "GeometricFunctions.hpp"

void TacticalDecisionSystem::Run(
    const std::map<Area::Id, Area> areas,
    RoutingEngine& routingEngine,
    std::vector<std::unique_ptr<Agent>>& agents) const
{

    for(auto& agent : agents) {
        const auto dest = agent->waypoint;
        const auto waypoints = routingEngine.ComputeWaypoint(agent->GetPos(), dest);
        agent->destination = waypoints[1];
    }
}
