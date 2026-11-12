// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "generic_agent.hpp"
#include "routing_engine.hpp"
#include "routing_target.hpp"

#include <algorithm>
#include <cstddef>
#include <variant>
#include <vector>

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

    /// Parallel variant, part one (serial): solves every floor-field destination in use once
    /// and routes agents heading for a plain location, whose shortest-path engine caches its
    /// last tree and so is not thread-safe. Part two is route_prepared(), per agent.
    void prepare(RoutingEngine& routing_engine, auto&& agents) const
    {
        std::vector<std::size_t> prepared{};
        for(auto& agent : agents) {
            if(const auto* d = std::get_if<Destination>(&agent.final_target)) {
                if(std::find(prepared.begin(), prepared.end(), d->id()) == prepared.end()) {
                    d->prepare();
                    prepared.push_back(d->id());
                }
            } else {
                agent.route_orientation =
                    routing_engine.get_orientation(agent.location, agent.final_target);
            }
        }
    }

    /// Routes, serially, only the agents heading for a plain location (shortest-path engine).
    void route_locations(RoutingEngine& routing_engine, auto&& agents) const
    {
        for(auto& agent : agents) {
            if(std::holds_alternative<Location>(agent.final_target)) {
                agent.route_orientation =
                    routing_engine.get_orientation(agent.location, agent.final_target);
            }
        }
    }

    /// Parallel variant, part two: routes one agent heading for a prepared destination. Only
    /// reads shared state and writes this agent, so agents may be routed concurrently.
    static void route_prepared(GenericAgent& agent)
    {
        if(const auto* d = std::get_if<Destination>(&agent.final_target)) {
            agent.route_orientation = d->prepared_orientation(agent.location);
        }
    }
};
