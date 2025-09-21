// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "RoutingEngine.hpp"

#include <GenericAgent.hpp>
#include <random>
class TacticalDecisionSystem
{
private:
    struct PathCache {
        Point cached_destination;
        Point last_pos;
        bool valid = false;
    };

    mutable std::unordered_map<GenericAgent::ID, PathCache> path_cache;

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
            auto& cache_entry = path_cache[agent_id];

            
            bool near_waypoint = Distance(agent.pos, cache_entry.cached_destination) < 0.5f;
            std::cout << "[TacticalDecision] Agent Position: (" << agent.pos.x << ", "
                      << agent.pos.y << ")" << std::endl;
            
            if(!cache_entry.valid || near_waypoint) {
                std::hash<GenericAgent::ID> hasher;
                uint32_t seed = static_cast<uint32_t>(hasher(agent_id));
                std::mt19937 agent_rng(seed);
                std::uniform_real_distribution<float> bias_dist(-0.8f, 0.8f);
                float agent_bias = bias_dist(agent_rng);

                cache_entry.cached_destination =
                    routingEngine.ComputeWaypoint(agent.pos, agent.target, agent_bias);
                cache_entry.valid = true;
                // --- Debug logging ---
 std::cout << "[TacticalDecision] Target: (" << agent.target.x << ", "
                          << agent.target.y << ")" << std::endl;
                std::cout << "[TacticalDecision] New Waypoint: ("
                          << cache_entry.cached_destination.x << ", "
                          << cache_entry.cached_destination.y << ")" << std::endl;
                std::cout << "----------------------------------------" << std::endl;
            }

            cache_entry.last_pos = agent.pos;
            agent.destination = cache_entry.cached_destination;
        }
    }
};
