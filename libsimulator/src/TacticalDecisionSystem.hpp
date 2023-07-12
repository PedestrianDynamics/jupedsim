/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

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

    void Run(RoutingEngine& routingEngine, std::vector<Agent>& agents) const;
};

template <typename Agent>
void TacticalDecisionSystem<Agent>::Run(RoutingEngine& routingEngine, std::vector<Agent>& agents)
    const
{
    for(auto& agent : agents) {
        const auto dest = agent.waypoint;
        agent.destination = routingEngine.ComputeWaypoint(agent.pos, dest);
    }
}
