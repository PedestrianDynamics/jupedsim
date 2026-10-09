// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine.hpp"

class TacticalDecisionSystem
{
public:
    TacticalDecisionSystem() = default;
    ~TacticalDecisionSystem() = default;
    TacticalDecisionSystem(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem& operator=(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem(TacticalDecisionSystem&& other) = delete;
    TacticalDecisionSystem& operator=(TacticalDecisionSystem&& other) = delete;

    void run(RoutingEngine& routing_engine, auto&& agents) const
    {
        for(auto& agent : agents) {
            agent.route_orientation =
                routing_engine.get_orientation(agent.location, agent.final_target);
        }
    }
};
