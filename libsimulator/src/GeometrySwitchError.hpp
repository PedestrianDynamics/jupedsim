// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GenericAgent.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include <fmt/core.h>
#include <fmt/format.h>

class GeometrySwitchError : public SimulationError
{
    std::vector<GenericAgent::ID> faultyAgents;
    std::vector<BaseStage::ID> faultyStages;

public:
    template <typename... Args>
    GeometrySwitchError(
        const char* msg,
        std::vector<GenericAgent::ID> faultyAgents_,
        std::vector<BaseStage::ID> faultyStages_,
        const Args&... args)
        : SimulationError(msg, args...), faultyAgents(faultyAgents_), faultyStages(faultyStages_)
    {
    }

    std::vector<GenericAgent::ID> FaultyAgents() const { return faultyAgents; }

    std::vector<BaseStage::ID> FaultyStages() const { return faultyStages; }
};
