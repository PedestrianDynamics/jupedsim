// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "geometry/location.hpp"
#include "kernel.hpp"
#include "routing_engine.hpp"

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

    void run(RoutingEngine& routing_engine, auto&& agents) const
    {
        for(auto& agent : agents) {
            orient(routing_engine, agent);
        }
    }

    /// Points the route orientation of @p agent at its target.
    static void orient(RoutingEngine& routing_engine, GenericAgent& agent)
    {
        const auto* place = std::get_if<Location>(&agent.final_target);
        agent.route_orientation =
            (agent.held || agent.stranded) && place != nullptr ?
                back_to(agent.location, *place) :
                routing_engine.get_orientation(agent.location, agent.final_target);
    }

    /// Straight from @p from to @p place, where a held or stranded agent stopped; zero once it
    /// stands there. Keeps these agents off the navigation mesh: each has a target of its own.
    static Vector back_to(const Location& from, const Location& place)
    {
        const auto offset = place.xy() - from.xy();
        return offset.is_zero_length() ? Vector{} : offset.normalized();
    }
};
