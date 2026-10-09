// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "StageManager.hpp"

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
        std::vector<GenericAgent::ID>& removed_agent_ids,
        StageManager& stage_manager) const;
};

template <typename Agent>
void AgentRemovalSystem<Agent>::run(
    AgentContainer<Agent>& agents,
    std::vector<GenericAgent::ID>& removed_agent_ids,
    StageManager& stage_manager) const
{

    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&removed_agent_ids, &stage_manager](const GenericAgent& agent) {
            auto found =
                std::find(std::begin(removed_agent_ids), std::end(removed_agent_ids), agent.id) !=
                std::end(removed_agent_ids);
            if(found) {
                stage_manager.handle_remove_agent(agent.stage_id);
            }
            return found;
        });
    agents.erase(iter, std::end(agents));

    removed_agent_ids.clear();
}
