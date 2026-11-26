// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"

#include <boost/container/deque.hpp>

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

    /// Removes the agents that leave the simulation, see GenericAgent::leaving.
    void run(AgentContainer<Agent>& agents, StageManager& stage_manager) const;
};

template <typename Agent>
void AgentRemovalSystem<Agent>::run(AgentContainer<Agent>& agents, StageManager& stage_manager)
    const
{
    boost::container::erase_if(agents, [&stage_manager](const GenericAgent& agent) {
        if(agent.leaving && agent.stage_id != BaseStage::ID::invalid) {
            stage_manager.handle_remove_agent(agent.stage_id);
        }
        return agent.leaving;
    });
}
