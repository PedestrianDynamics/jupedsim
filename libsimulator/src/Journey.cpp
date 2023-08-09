// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include "Events.hpp"
#include "GenericAgent.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "TemplateHelper.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "fmt/ranges.h"

////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////
Journey::Journey(std::vector<Stage*> stages_) : stages(std::move(stages_))
{
}

std::tuple<Point, size_t, Stage::ID> Journey::Target(const GenericAgent& agent) const
{
    for(size_t idx = agent.currentJourneyStageIdx; idx < stages.size(); ++idx) {
        if(stages[idx]->IsCompleted(agent)) {
            continue;
        }
        return std::make_tuple(stages[idx]->Target(agent), idx, stages[idx]->Id());
    }
    return std::make_tuple(stages.back()->Target(agent), stages.size() - 1, stages.back()->Id());
}
