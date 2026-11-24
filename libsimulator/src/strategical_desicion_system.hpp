// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "geometry/location.hpp"
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

    /// Marks the agents in @p agents that reached their target, a stage of their route or the
    /// location they are steered to. An agent reaching an exit leaves: it is added to @p leaving,
    /// to be removed at the beginning of the next iteration.
    template <typename Agents>
    void mark_reached(
        Agents&& agents,
        const StageManager& stage_manager,
        std::vector<GenericAgent::ID>& leaving) const
    {
        for(auto& agent : agents) {
            if(agent.target_reached) {
                continue;
            }
            if(agent.route_id == Route::ID::invalid) {
                const auto& target = std::get<Location>(agent.current_target);
                if(has_reached(agent.location, target)) {
                    agent.target_reached = true;
                }
                continue;
            }
            const auto* stage = stage_manager.stage(agent.stage_id);
            if(!stage->is_reached_by(agent)) {
                continue;
            }
            agent.target_reached = true;
            if(dynamic_cast<const Exit*>(stage) != nullptr) {
                agent.leaving = true;
                leaving.push_back(agent.id);
            }
        }
    }

    /// Moves the agents that reached a stage of their route on to the next one. An agent stays
    /// at the last stage of its route.
    void
    run(const std::unordered_map<Route::ID, std::unique_ptr<Route>>& routes,
        AgentContainer<GenericAgent>& agents,
        StageManager& stage_manager) const
    {
        for(auto& agent : agents) {
            if(!agent.target_reached || agent.leaving || agent.route_id == Route::ID::invalid) {
                continue;
            }
            const auto* next = routes.at(agent.route_id)->next_stage(agent.stage_id);
            if(next == nullptr) {
                continue;
            }
            stage_manager.migrate_agent(agent.stage_id, next->id());
            agent.stage_id = next->id();
            agent.current_target = next->destination();
            agent.target_reached = false;
        }
    }
};
