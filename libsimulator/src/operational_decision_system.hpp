// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "agent_view.hpp"
#include "environment_query.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "job_queue.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"

#include <algorithm>
#include <cstddef>
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

    OperationalModelType model_type() const { return _model->type(); }

    void
    run(double dt,
        double /*t_in_sec*/,
        const NeighborhoodSearch<GenericAgent>& neighborhood_search,
        const Geometry& geometry,
        AgentContainer<GenericAgent>& agents,
        JobQueue& jobs)
    {
        const EnvironmentQuery env_query{geometry, neighborhood_search};
        _next.clear();
        std::copy(std::begin(agents), std::end(agents), std::back_inserter(_next));
        // Each step reads only the current generation and writes only its own slot in
        // _next, so steps can run in any order and at the same time.
        const auto compute_step = [&](size_t index) {
            const auto& current = agents[index];
            auto& next = _next[index];
            const AgentStep step{env_query, current, dt};
            const Vector movement = _model->compute_next_state(current.state, next.state, step);
            next.location.move_on_surface(movement);
        };
        if(_model->supports_parallel_step()) {
            parallel_for(jobs, agents.size(), compute_step);
        } else {
            for(size_t index = 0; index < agents.size(); ++index) {
                compute_step(index);
            }
        }
        // Swap in the computed generation. This is safe because no caller retains
        // pointers/references across an iteration (Python-side agent handles resolve per
        // access) and Simulation::iterate rebuilds the neighborhood grid right after this
        // step.
        agents.swap(_next);
    }

    void validate_agent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhood_search,
        const Geometry& geometry) const
    {
        const EnvironmentQuery env_query{geometry, neighborhood_search};
        _model->check_model_constraint(agent, AgentView{env_query, agent});
    }

    void seed_agent_state(OperationalModelState& state) { _model->seed_agent_state(state); }
};
