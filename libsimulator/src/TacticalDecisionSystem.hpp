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
        int stuck_counter = 0;
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

            // Check if agent has reached its current waypoint
            bool near_waypoint =
                !cache_entry.valid || Distance(agent.pos, cache_entry.cached_destination) < 0.5f;

            // Detect if agent is stuck
            bool making_progress =
                !cache_entry.valid || Distance(agent.pos, cache_entry.last_pos) > 0.05f;

            if(!making_progress) {
                cache_entry.stuck_counter++;
            } else {
                cache_entry.stuck_counter = 0;
            }

            bool force_recalc = cache_entry.stuck_counter > 10;

            if(near_waypoint || force_recalc) {
                std::hash<GenericAgent::ID> hasher;
                uint32_t seed = static_cast<uint32_t>(hasher(agent_id));
                if(force_recalc) {
                    seed += cache_entry.stuck_counter;
                }

                std::mt19937 agent_rng(seed);
                std::uniform_real_distribution<float> bias_dist(-0.8f, 0.8f);
                float agent_bias = bias_dist(agent_rng);

                cache_entry.cached_destination =
                    routingEngine.ComputeWaypoint(agent.pos, agent.target, agent_bias);
                cache_entry.stuck_counter = 0;
                cache_entry.valid = true;
            }

            cache_entry.last_pos = agent.pos;
            agent.destination = cache_entry.cached_destination;
        }
    }
};
