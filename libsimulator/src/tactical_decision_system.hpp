// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "job_queue.hpp"
#include "routing_engine.hpp"

#include <cstddef>
#include <iterator>
#include <variant>

class TacticalDecisionSystem
{
public:
    TacticalDecisionSystem() = default;
    ~TacticalDecisionSystem() = default;
    TacticalDecisionSystem(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem& operator=(const TacticalDecisionSystem& other) = delete;
    TacticalDecisionSystem(TacticalDecisionSystem&& other) = delete;
    TacticalDecisionSystem& operator=(TacticalDecisionSystem&& other) = delete;

    void run(RoutingEngine& routing_engine, auto&& agents, JobQueue& jobs) const
    {
        // Floor fields are solved on first use and the shortest path engine caches its last
        // tree, so both stay on this thread. What is left only reads solved floor fields.
        for(auto& agent : agents) {
            if(const auto* destination = std::get_if<Destination>(&agent.final_target)) {
                destination->solve();
            } else {
                agent.route_orientation =
                    routing_engine.get_orientation(agent.location, agent.final_target);
            }
        }
        const auto first = std::begin(agents);
        const auto count = static_cast<std::size_t>(std::distance(first, std::end(agents)));
        parallel_for(jobs, count, [first](std::size_t index) {
            auto& agent = first[index];
            if(const auto* destination = std::get_if<Destination>(&agent.final_target)) {
                agent.route_orientation = destination->solved_orientation(agent.location);
            }
        });
    }
};
