/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Area.hpp"
#include "GenericAgent.hpp"

#include <map>
#include <vector>

class AgentExitSystem
{
public:
    AgentExitSystem() = default;
    ~AgentExitSystem() = default;
    AgentExitSystem(const AgentExitSystem& other) = delete;
    AgentExitSystem& operator=(const AgentExitSystem& other) = delete;
    AgentExitSystem(AgentExitSystem&& other) = delete;
    AgentExitSystem& operator=(AgentExitSystem&& other) = delete;

    void
    Run(const std::map<Area::Id, Area> areas,
        std::vector<GenericAgent>& agents,
        std::vector<uint64_t>& removedAgentIds) const;
};
