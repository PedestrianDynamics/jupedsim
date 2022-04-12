#pragma once

#include "Area.hpp"
#include "pedestrian/Pedestrian.hpp"

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
        std::vector<std::unique_ptr<Pedestrian>>& agents) const;
};
