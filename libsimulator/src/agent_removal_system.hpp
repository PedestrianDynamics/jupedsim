// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "records.hpp"
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

    /// Removes the agents of @p removals, which have to be sorted by agent id.
    void
    run(AgentContainer<Agent>& agents,
        const std::vector<RemovalRecord>& removals,
        StageManager& stage_manager) const;
};

template <typename Agent>
void AgentRemovalSystem<Agent>::run(
    AgentContainer<Agent>& agents,
    const std::vector<RemovalRecord>& removals,
    StageManager& stage_manager) const
{
    if(removals.empty()) {
        return;
    }
    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&removals, &stage_manager](const GenericAgent& agent) {
            const bool found =
                std::ranges::binary_search(removals, agent.id, {}, &RemovalRecord::agent_id);
            if(found && agent.stage_id != BaseStage::ID::invalid) {
                stage_manager.handle_remove_agent(agent.stage_id);
            }
            return found;
        });
    agents.erase(iter, std::end(agents));
}
