// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry/Geometry2D.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

class OperationalDecisionSystem
{
    std::unique_ptr<OperationalModel> _model{};
    AgentContainer<GenericAgent> _next{};

public:
    OperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model) : _model(std::move(model))
    {
    }
    ~OperationalDecisionSystem() = default;
    OperationalDecisionSystem(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem& operator=(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem(OperationalDecisionSystem&& other) = delete;
    OperationalDecisionSystem& operator=(OperationalDecisionSystem&& other) = delete;

    OperationalModelType ModelType() const { return _model->Type(); }

    void
    Run(double dT,
        double /*t_in_sec*/,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const Geometry2D& geometry,
        AgentContainer<GenericAgent>& agents)
    {
        _next.clear();
        std::copy(std::begin(agents), std::end(agents), std::back_inserter(_next));
        for(size_t index = 0; index < agents.size(); ++index) {
            auto& next = _next[index];
            // `next` starts as a copy of the current agent, so its Location holds the
            // pre-step (x,y) and cached face -- the straight walk starts from there.
            const bool hasLocation = next.location.has_value();
            const Point before = hasLocation ? next.location->xy() : Point{};
            _model->ComputeNextState(dT, agents[index], next, geometry, neighborhoodSearch);
            if(hasLocation) {
                // Advance the Location by the model's xy change.
                // Basically we redo the walk on 3D mesh. This will get the real movement later,
                // so that this part gets removed.
                next.location->move_on_surface(next.position() - before);
            }
        }
        // Swap in the computed generation. This is safe because no caller retains
        // pointers/references across an iteration (Python-side agent handles resolve per
        // access) and Simulation::Iterate rebuilds the neighborhood grid right after this
        // step.
        agents.swap(_next);
    }

    void ValidateAgent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const Geometry2D& geometry) const
    {
        _model->CheckModelConstraint(agent, neighborhoodSearch, geometry);
    }
};
