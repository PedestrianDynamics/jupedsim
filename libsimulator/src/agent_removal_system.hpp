// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "stage.hpp"
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

    /// Removes the agents with a record in @p removed_agents or @p marked. Afterwards
    /// @p removed_agents holds the records of @p marked, which is left empty.
    void
    run(AgentContainer<Agent>& agents,
        std::vector<RemovalRecord>& removed_agents,
        std::vector<RemovalRecord>& marked,
        StageManager& stage_manager) const;
};

template <typename Agent>
void AgentRemovalSystem<Agent>::run(
    AgentContainer<Agent>& agents,
    std::vector<RemovalRecord>& removed_agents,
    std::vector<RemovalRecord>& marked,
    StageManager& stage_manager) const
{
    const auto has_record = [](const std::vector<RemovalRecord>& records, const Agent& agent) {
        return std::any_of(std::begin(records), std::end(records), [&agent](const auto& record) {
            return record.id == agent.id;
        });
    };

    auto iter = std::remove_if(std::begin(agents), std::end(agents), [&](const Agent& agent) {
        const auto found = has_record(removed_agents, agent) || has_record(marked, agent);
        if(found) {
            stage_manager.handle_remove_agent(agent.stage_id);
        }
        return found;
    });
    agents.erase(iter, std::end(agents));

    removed_agents.clear();
    removed_agents.swap(marked);
}
