#pragma once

#include "Area.hpp"
#include "Pedestrian.hpp"

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

    void
    Run(const std::map<Area::Id, Area>& areas,
        std::vector<std::unique_ptr<Pedestrian>>& agents) const;
};
