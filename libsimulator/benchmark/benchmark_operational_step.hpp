// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "collision_free_speed_model_v2_state.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/geometry_builder.hpp"
#include "job_queue.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "operational_decision_system.hpp"
#include "operational_models/collision_free_speed_model_v2/collision_free_speed_model_v2.hpp"
#include "operational_models/social_force_model/social_force_model.hpp"
#include "social_force_model_state.hpp"
#include "unique_id.hpp"

#include <benchmark/benchmark.h>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace operational_step_benchmark
{
constexpr double spacing = 0.8;
/// Free space between the crowd and the walls. Agents within the models' wall search radius
/// (4 m) run the exact wall query, which dominates a step; `open_margin` keeps every agent out
/// of its reach, so a step costs only the model itself.
constexpr double wall_margin = 2.0;
constexpr double open_margin = 5.0;
constexpr double dt = 0.01;
/// Agents walk towards a wall, so they are put back on their grid every so many steps.
constexpr int steps_before_reset = 50;

/// A square room just large enough for `agent_count` agents on a grid.
inline std::unique_ptr<Geometry> room_for(std::size_t agent_count, double margin)
{
    const auto per_row = static_cast<std::size_t>(std::ceil(std::sqrt(agent_count)));
    const double side = static_cast<double>(per_row) * spacing + 2 * margin;
    GeometryBuilder builder{};
    builder.add_region({{{0, 0}, {side, 0}, {side, side}, {0, side}}, {}}, 0.0);
    return builder.build();
}

/// `agent_count` agents on a grid, all heading in +x.
template <typename State>
AgentContainer<GenericAgent>
agent_grid(const Geometry& geometry, std::size_t agent_count, double margin, const State& state)
{
    const auto per_row = static_cast<std::size_t>(std::ceil(std::sqrt(agent_count)));
    AgentContainer<GenericAgent> agents{};
    for(std::size_t index = 0; index < agent_count; ++index) {
        const double x = margin + static_cast<double>(index % per_row) * spacing;
        const double y = margin + static_cast<double>(index / per_row) * spacing;
        auto& agent = agents.emplace_back(
            GenericAgent::ID::invalid,
            jps::UniqueID<Journey>::invalid,
            jps::UniqueID<BaseStage>::invalid,
            *geometry.get_location_near_z(x, y, 0.0),
            state);
        agent.route_orientation = Vector{1.0, 0.0};
    }
    return agents;
}

/// Times only OperationalDecisionSystem::run; rebuilding the neighborhood grid and resetting
/// the crowd happen off the clock. Arguments: agent count, worker count, whether the crowd
/// starts next to the walls.
template <typename Model, typename State>
void run_steps(benchmark::State& bench, std::unique_ptr<Model> model, const State& state)
{
    const auto agent_count = static_cast<std::size_t>(bench.range(0));
    const auto worker_count = static_cast<std::size_t>(bench.range(1));
    const double margin = bench.range(2) != 0 ? wall_margin : open_margin;
    const auto geometry = room_for(agent_count, margin);
    const auto initial = agent_grid(*geometry, agent_count, margin, state);
    auto agents = initial;
    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    OperationalDecisionSystem system{std::move(model)};
    JobQueue jobs{worker_count};

    int step = 0;
    for(auto _ : bench) {
        if(step++ % steps_before_reset == 0) {
            agents = initial;
        }
        neighborhood_search.update(agents);
        const auto start = std::chrono::steady_clock::now();
        system.run(dt, 0.0, neighborhood_search, *geometry, agents, jobs);
        const auto end = std::chrono::steady_clock::now();
        bench.SetIterationTime(std::chrono::duration<double>(end - start).count());
    }
    bench.SetItemsProcessed(bench.iterations() * static_cast<int64_t>(agent_count));
}

inline void sweep(benchmark::Benchmark* bench)
{
    bench->ArgNames({"agents", "workers", "walls"});
    for(const int64_t walls : {0, 1}) {
        for(const int64_t agents : {100, 1000, 10000}) {
            for(const int64_t workers : {0, 1, 3, 5, 9}) {
                bench->Args({agents, workers, walls});
            }
        }
    }
    bench->UseManualTime()->Unit(benchmark::kMicrosecond);
}
} // namespace operational_step_benchmark

static void bm_operational_step_collision_free_speed_v2(benchmark::State& bench)
{
    CollisionFreeSpeedModelV2State state{};
    state.orientation = Vector{1.0, 0.0};
    operational_step_benchmark::run_steps(
        bench, std::make_unique<CollisionFreeSpeedModelV2>(), state);
}
BENCHMARK(bm_operational_step_collision_free_speed_v2)->Apply(operational_step_benchmark::sweep);

static void bm_operational_step_social_force(benchmark::State& bench)
{
    SocialForceModelState state{};
    operational_step_benchmark::run_steps(
        bench, std::make_unique<SocialForceModel>(120000.0, 240000.0), state);
}
BENCHMARK(bm_operational_step_social_force)->Apply(operational_step_benchmark::sweep);
