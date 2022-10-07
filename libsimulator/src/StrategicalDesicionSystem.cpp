/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "StrategicalDesicionSystem.hpp"

#include <stdexcept>

void StrategicalDecisionSystem::Run(
    const std::map<Area::Id, Area>& areas,
    std::vector<GenericAgent>& agents) const
{
    for(auto& agent : agents) {
        agent.waypoint = agent.behaviour->DesiredDestination(agent.pos);
    }
}
