// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "stage_manager.hpp"

#include <algorithm>
#include <map>
#include <vector>

template <typename Agent>
class AgentRemovalSystem
{
public:
    AgentRemovalSystem() = default;
    ~AgentRemovalSystem() = default;
    AgentRemovalSystem(const AgentRemovalSystem& other) = delete;
    AgentRemovalSystem& operator=(const AgentRemovalSystem& other) = delete;
    AgentRemovalSystem(AgentRemovalSystem&& other) = delete;
    AgentRemovalSystem& operator=(AgentRemovalSystem&& other) = delete;

    void
    run(AgentContainer<Agent>& agents,
        std::vector<RemovalRecord>& removed_agents,
        StageManager& stage_manager) const;
};

template <typename Agent>
void AgentRemovalSystem<Agent>::run(
    AgentContainer<Agent>& agents,
    std::vector<RemovalRecord>& removed_agents,
    StageManager& stage_manager) const
{

    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&removed_agents, &stage_manager](const GenericAgent& agent) {
            auto found = std::any_of(
                std::begin(removed_agents), std::end(removed_agents), [&agent](const auto& record) {
                    return record.id == agent.id;
                });
            if(found) {
                stage_manager.handle_remove_agent(agent.stage_id);
            }
            return found;
        });
    agents.erase(iter, std::end(agents));

    removed_agents.clear();
}
