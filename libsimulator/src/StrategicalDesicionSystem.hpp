/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Area.hpp"
#include "GenericAgent.hpp"
#include "Point.hpp"

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

    void Run(const std::map<Area::Id, Area>& areas, std::vector<GenericAgent>& agents) const;
};
