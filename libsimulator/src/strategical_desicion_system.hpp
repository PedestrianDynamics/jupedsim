// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "route.hpp"
#include "routing_engine.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>

class StrategicalDecisionSystem
{
public:
    StrategicalDecisionSystem() = default;
    ~StrategicalDecisionSystem() = default;
    StrategicalDecisionSystem(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem& operator=(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem(StrategicalDecisionSystem&& other) = delete;
    StrategicalDecisionSystem& operator=(StrategicalDecisionSystem&& other) = delete;

    void
    run(const std::unordered_map<Route::ID, std::unique_ptr<Route>>& routes,
        auto&& agents,
        StageManager& stage_manager,
        const RoutingEngine& routing_engine,
        uint64_t iteration) const
    {
        for(auto& agent : agents) {
            if(agent.stranded || agent.held) {
                continue;
            }
            const auto [next, decision_stage, held] =
                routes.at(agent.route_id)->step(agent, routing_engine);
            agent.decision_stage = decision_stage;
            if(held) {
                hold(agent, iteration);
                continue;
            }
            if(next == nullptr) {
                strand(agent);
                continue;
            }
            assign(agent, *next, stage_manager);
        }
    }

    /// Sends @p agent to @p stage.
    static void assign(GenericAgent& agent, BaseStage& stage, StageManager& stage_manager)
    {
        agent.final_target = stage.target(agent);
        stage_manager.migrate_agent(agent.stage_id, stage.id());
        agent.stage_id = stage.id();
    }

    /// Stops @p agent where it stands; it keeps its stage.
    static void strand(GenericAgent& agent)
    {
        agent.stranded = true;
        agent.final_target = agent.location;
    }

    /// Stops @p agent where it stands at its held stage, from @p iteration on.
    static void hold(GenericAgent& agent, uint64_t iteration)
    {
        agent.held = true;
        agent.held_since = iteration;
        agent.final_target = agent.location;
    }
};
