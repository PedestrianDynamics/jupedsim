// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "agent_view.hpp"
#include "environment_query.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "thread_pool.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <memory>
#include <utility>

class OperationalDecisionSystem
{
    std::unique_ptr<OperationalModel> _model{};
    AgentContainer<GenericAgent> _next{};
    std::unique_ptr<ThreadPool> _pool{};

public:
    OperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model) : _model(std::move(model))
    {
    }
    OperationalDecisionSystem(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem& operator=(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem(OperationalDecisionSystem&& other) = delete;
    OperationalDecisionSystem& operator=(OperationalDecisionSystem&& other) = delete;

    OperationalModelType model_type() const { return _model->type(); }

    /// Threads used for the operational step, including the calling one. 1 runs serially.
    void set_num_threads(std::size_t threads)
    {
        _pool = threads > 1 ? std::make_unique<ThreadPool>(threads) : nullptr;
    }

    std::size_t num_threads() const { return _pool ? _pool->size() : 1; }

    /// The pool, shared with the other per-agent phases; null when running serially.
    ThreadPool* pool() const { return _pool.get(); }

    void
    run(double dt,
        std::uint64_t iteration,
        const NeighborhoodSearch<GenericAgent>& neighborhood_search,
        const Geometry& geometry,
        AgentContainer<GenericAgent>& agents,
        const std::function<void(GenericAgent&)>& before_step = {},
        std::vector<Grid2DIndex>* next_cells = nullptr)
    {
        const EnvironmentQuery env_query{geometry, neighborhood_search};
        const auto t_copy = std::chrono::steady_clock::now();
        // _next holds the generation before last; when the agent count is unchanged each slot is
        // overwritten inside the (parallel) loop, so every thread copies only its own agents.
        const bool copy_in_loop = _next.size() == agents.size();
        if(!copy_in_loop) {
            _next.assign(std::begin(agents), std::end(agents));
        }
        stat_copy_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now() - t_copy)
                            .count();
        // Every agent reads only the current generation and writes only its own slot of the
        // next one, so agents are independent within a step.
        const auto step_agent = [&](std::size_t index) {
            if(before_step) {
                before_step(agents[index]);
            }
            const auto& current = agents[index];
            auto& next = _next[index];
            if(copy_in_loop) {
                next = current;
            } else {
                next.route_orientation = current.route_orientation;
            }
            const AgentStep step{env_query, current, dt, nullptr, iteration, index};
            const Vector movement = _model->compute_next_state(current.state, next.state, step);
            next.location.move_on_surface(movement);
            if(next_cells != nullptr) {
                (*next_cells)[index] = neighborhood_search.get_index(next.location.xy());
            }
        };
        if(_pool && _model->is_thread_safe()) {
            // SPIKE: JPS_CHUNK overrides the chunk size; 0 means one contiguous block per thread.
            static const long chunk_env = [] {
                const char* env = std::getenv("JPS_CHUNK");
                return env != nullptr ? std::strtol(env, nullptr, 10) : -1L;
            }();
            // About 16 chunks per thread: small enough to balance agents near walls (several
            // times the cost of others), large enough to keep the shared counter cold.
            std::size_t chunk = std::max<std::size_t>(1, agents.size() / (_pool->size() * 16));
            if(chunk_env == 0) {
                chunk = (agents.size() + _pool->size() - 1) / _pool->size();
            } else if(chunk_env > 0) {
                chunk = static_cast<std::size_t>(chunk_env);
            }
            _pool->parallel_for(agents.size(), chunk, step_agent);
        } else {
            for(std::size_t index = 0; index < agents.size(); ++index) {
                step_agent(index);
            }
        }
        // Swap in the computed generation. This is safe because no caller retains
        // pointers/references across an iteration (Python-side agent handles resolve per
        // access) and Simulation::iterate rebuilds the neighborhood grid right after this
        // step.
        const auto t_swap = std::chrono::steady_clock::now();
        agents.swap(_next);
        stat_swap_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now() - t_swap)
                            .count();
    }

    // SPIKE diagnostics
    std::uint64_t stat_copy_ns{0};
    std::uint64_t stat_swap_ns{0};
    ~OperationalDecisionSystem()
    {
        if(std::getenv("JPS_POOL_STATS") != nullptr) {
            std::fprintf(
                stderr,
                "ODS copy=%.3fs swap=%.3fs sizeof(GenericAgent)=%zu\n",
                static_cast<double>(stat_copy_ns) / 1e9,
                static_cast<double>(stat_swap_ns) / 1e9,
                sizeof(GenericAgent));
        }
    }

    void validate_agent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhood_search,
        const Geometry& geometry) const
    {
        const EnvironmentQuery env_query{geometry, neighborhood_search};
        _model->check_model_constraint(agent, AgentView{env_query, agent});
    }
};
