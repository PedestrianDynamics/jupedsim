// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "route.hpp"
#include "routing_engine.hpp"
#include "stage_manager.hpp"

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
        const RoutingEngine& routing_engine) const
    {
        for(auto& agent : agents) {
            const auto [target, id] = routes.at(agent.route_id)->target(agent, routing_engine);
            agent.final_target = target;
            stage_manager.migrate_agent(agent.stage_id, id);
            agent.stage_id = id;
        }
    }
};
