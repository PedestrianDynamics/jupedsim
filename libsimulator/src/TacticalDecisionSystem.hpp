// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine.hpp"

#include <random>

class TacticalDecisionSystem
{
public:
    TacticalDecisionSystem() = default;
    ~TacticalDecisionSystem() = default;
    TacticalDecisionSystem(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem& operator=(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem(TacticalDecisionSystem&& other) = delete;
    TacticalDecisionSystem& operator=(TacticalDecisionSystem&& other) = delete;

    void Run(RoutingEngine& routingEngine, auto&& agents) const
    {
        for(auto& agent : agents) {
            auto agent_id = agent.id;
            std::hash<decltype(agent_id)> hasher;
            uint32_t seed = static_cast<uint32_t>(hasher(agent_id));
            std::mt19937 agent_rng(seed); // Seed with agent ID for consistency
            std::uniform_real_distribution<float> bias_dist(-0.8f, 0.8f);
            float agent_bias = bias_dist(agent_rng);
            const auto dest = agent.target;
            agent.destination = routingEngine.ComputeWaypoint(agent.pos, dest, agent_bias);
        }
    }
};
