/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Area.hpp"
#include "RoutingEngine.hpp"

#include <vector>

template <typename Agent>
class TacticalDecisionSystem
{
public:
    TacticalDecisionSystem() = default;
    ~TacticalDecisionSystem() = default;
    TacticalDecisionSystem(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem& operator=(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem(TacticalDecisionSystem&& other) = delete;
    TacticalDecisionSystem& operator=(TacticalDecisionSystem&& other) = delete;

    void
    Run(const std::map<Area::Id, Area> areas,
        RoutingEngine& routingEngine,
        std::vector<Agent>& agents) const;
};

template <typename Agent>
void TacticalDecisionSystem<Agent>::Run(
    const std::map<Area::Id, Area> areas,
    RoutingEngine& routingEngine,
    std::vector<Agent>& agents) const
{

    for(auto& agent : agents) {
        const auto dest = agent.waypoint;
        const auto waypoints = routingEngine.ComputeWaypoint(agent.pos, dest);
        agent.destination = waypoints[1];
    }
}
