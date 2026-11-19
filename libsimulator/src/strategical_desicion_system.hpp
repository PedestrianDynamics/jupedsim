// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "geometry/location.hpp"
#include "records.hpp"
#include "route.hpp"
#include "routing_target.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class StrategicalDecisionSystem
{
public:
    StrategicalDecisionSystem() = default;
    ~StrategicalDecisionSystem() = default;
    StrategicalDecisionSystem(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem& operator=(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem(StrategicalDecisionSystem&& other) = delete;
    StrategicalDecisionSystem& operator=(StrategicalDecisionSystem&& other) = delete;

    /// Completes the targets agents have reached, recorded in @p completions, and moves them on
    /// along their routes. An agent completing an exit is recorded in @p removals and goes to
    /// @p to_remove instead.
    void
    run(const std::unordered_map<Route::ID, std::unique_ptr<Route>>& routes,
        AgentContainer<GenericAgent>& agents,
        StageManager& stage_manager,
        double time,
        std::vector<TargetCompletionRecord>& completions,
        std::vector<RemovalRecord>& removals,
        std::vector<GenericAgent::ID>& to_remove) const
    {
        for(auto& agent : agents) {
            if(agent.target_completed) {
                continue;
            }
            if(agent.route_id == Route::ID::invalid) {
                const auto& target = std::get<Location>(agent.final_target);
                if(has_reached(agent.location, target)) {
                    agent.target_completed = true;
                    completions.push_back({agent.id, target, time, agent.location});
                }
                continue;
            }
            const auto* stage = stage_manager.stage(agent.stage_id);
            if(!stage->is_completed(agent)) {
                continue;
            }
            completions.push_back({agent.id, stage->id(), time, agent.location});
            if(dynamic_cast<const Exit*>(stage) != nullptr) {
                agent.target_completed = true;
                agent.leaving = true;
                removals.push_back({agent.id, stage->id(), time});
                to_remove.push_back(agent.id);
                continue;
            }
            const auto* next = routes.at(agent.route_id)->next_stage(stage->id());
            if(next == nullptr) {
                agent.target_completed = true;
                continue;
            }
            stage_manager.migrate_agent(agent.stage_id, next->id());
            agent.stage_id = next->id();
            agent.final_target = next->destination();
        }
    }
};
