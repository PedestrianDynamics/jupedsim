// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine3D.hpp"

class TacticalDecisionSystem
{
public:
    TacticalDecisionSystem() = default;
    ~TacticalDecisionSystem() = default;
    TacticalDecisionSystem(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem& operator=(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem(TacticalDecisionSystem&& other) = delete;
    TacticalDecisionSystem& operator=(TacticalDecisionSystem&& other) = delete;

    void Run(RoutingEngine3D& routingEngine, auto&& agents) const
    {
        for(auto& agent : agents) {
            agent.nextTarget = routingEngine.ComputeWaypoint(agent.location, agent.finalTarget);
        }
    }
};
