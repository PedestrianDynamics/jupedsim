// SPDX-License-Identifier: LGPL-3.0-or-later
#include "agent_view.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry_fixtures.hpp"
#include "job_queue.hpp"
#include "kernel.hpp"
#include "neighborhood_search.hpp"
#include "operational_decision_system.hpp"
#include "operational_model_state.hpp"
#include "operational_models/custom_model/custom_model.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"
#include "unique_id.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>

namespace
{
struct ConstantVelocity {
    Vector velocity{};
};

/// Minimal model: advances the position by `velocity * dt` each step. Enough to
/// exercise the Location change in OperationalDecisionSystem::run.
class ConstantVelocityModel : public CustomModel
{
public:
    Vector compute_next_state(
        const OperationalModelState& current,
        OperationalModelState&,
        const AgentStep& step) const override
    {
        const auto& cur = std::get<CustomModel::State>(current);
        return cur.get<ConstantVelocity>().velocity * step.dt();
    }

    void check_model_constraint(const GenericAgent&, const AgentView&) const override {}
};

/// ConstantVelocityModel that declares itself safe to step agents concurrently.
class ParallelConstantVelocityModel : public ConstantVelocityModel
{
public:
    bool supports_parallel_step() const override { return true; }
};

/// Fails for agents moving with a negative x velocity.
class FailingModel : public ParallelConstantVelocityModel
{
public:
    Vector compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override
    {
        const auto& cur = std::get<CustomModel::State>(current);
        if(cur.get<ConstantVelocity>().velocity.x() < 0) {
            throw SimulationError("model failed");
        }
        return ParallelConstantVelocityModel::compute_next_state(current, next, step);
    }
};

/// Flags any step that runs on another thread than the one which created the model.
class ThreadRecordingModel : public ConstantVelocityModel
{
    std::thread::id _creator{std::this_thread::get_id()};
    std::atomic<bool>& _left_creator;

public:
    explicit ThreadRecordingModel(std::atomic<bool>& left_creator) : _left_creator(left_creator) {}

    Vector compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override
    {
        if(std::this_thread::get_id() != _creator) {
            _left_creator = true;
        }
        return ConstantVelocityModel::compute_next_state(current, next, step);
    }
};

GenericAgent make_agent(const Geometry& geometry, Point start, Vector velocity)
{
    return GenericAgent(
        GenericAgent::ID::invalid,
        jps::UniqueID<Journey>::invalid,
        jps::UniqueID<BaseStage>::invalid,
        *geometry.get_location_near_z(start.x(), start.y(), 0.0),
        CustomModel::State{ConstantVelocity{velocity}});
}

/// A flat square [-10,10]^2 with its 3D twin over the same polygon.
std::unique_ptr<Geometry> flat_square()
{
    return test_geometries::rectangle({-10, -10}, {10, 10}).geometry;
}

/// A 20 x 20 grid of agents with velocities that differ per agent.
AgentContainer<GenericAgent> agent_grid(const Geometry& geometry)
{
    AgentContainer<GenericAgent> agents{};
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {
            agents.emplace_back(make_agent(
                geometry,
                Point{-9.5 + i * 0.9, -9.5 + j * 0.9},
                Vector{0.1 * (j % 5), 0.05 * (i % 3)}));
        }
    }
    return agents;
}
} // namespace

TEST(OperationalDecisionSystemLocation, WalkingTheStepLandsWhereDeadReckoningSays)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{0.0, 0.0};
    agents.emplace_back(make_agent(*geo, start, Vector{2.0, 0.0}));

    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    neighborhood_search.update(agents);

    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};
    JobQueue jobs{0};
    system.run(0.5, 0.0, neighborhood_search, *geo, agents, jobs);

    // velocity=2.0, dt=0.5 --> move x by 1.0
    const auto& agent = agents.front();
    EXPECT_NEAR(agent.location.xy().x(), start.x() + 1.0, 1e-9);
    EXPECT_NEAR(agent.location.xy().y(), start.y(), 1e-9);
    EXPECT_NEAR(agent.location.z(), 0.0, 1e-9);
    EXPECT_EQ(agent.location.region(), 0u);
}

TEST(OperationalDecisionSystemLocation, ItStaysOnTheSheetOverManySteps)
{
    auto geo = flat_square();

    AgentContainer<GenericAgent> agents{};
    const Point start{-5.0, -3.0};
    const Vector velocity{1.0, 0.5};
    const double dt = 0.1;
    const int steps = 20;
    agents.emplace_back(make_agent(*geo, start, velocity));

    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    OperationalDecisionSystem system{std::make_unique<ConstantVelocityModel>()};
    JobQueue jobs{0};

    for(int step = 0; step < steps; ++step) {
        neighborhood_search.update(agents);
        system.run(dt, 0.0, neighborhood_search, *geo, agents, jobs);
        const auto& agent = agents.front();
        EXPECT_NEAR(agent.location.z(), 0.0, 1e-9);
        EXPECT_EQ(agent.location.region(), 0u);
    }
    // After `steps` steps the agent advanced by velocity * dt * steps.
    EXPECT_NEAR(agents.front().location.xy().x(), start.x() + velocity.x() * dt * steps, 1e-9);
    EXPECT_NEAR(agents.front().location.xy().y(), start.y() + velocity.y() * dt * steps, 1e-9);
}

TEST(OperationalDecisionSystemParallel, WorkersComputeTheSameStepsAsTheCallingThread)
{
    auto geo = flat_square();
    auto serial_agents = agent_grid(*geo);
    auto parallel_agents = agent_grid(*geo);

    NeighborhoodSearch<GenericAgent> serial_search{2.2};
    NeighborhoodSearch<GenericAgent> parallel_search{2.2};
    OperationalDecisionSystem serial{std::make_unique<ParallelConstantVelocityModel>()};
    OperationalDecisionSystem parallel{std::make_unique<ParallelConstantVelocityModel>()};
    JobQueue no_workers{0};
    JobQueue workers{3};

    for(int step = 0; step < 10; ++step) {
        serial_search.update(serial_agents);
        parallel_search.update(parallel_agents);
        serial.run(0.1, 0.0, serial_search, *geo, serial_agents, no_workers);
        parallel.run(0.1, 0.0, parallel_search, *geo, parallel_agents, workers);
    }

    ASSERT_EQ(serial_agents.size(), parallel_agents.size());
    for(std::size_t index = 0; index < serial_agents.size(); ++index) {
        EXPECT_EQ(serial_agents[index].location.xy(), parallel_agents[index].location.xy());
    }
}

TEST(OperationalDecisionSystemParallel, AModelErrorReachesTheCaller)
{
    auto geo = flat_square();
    AgentContainer<GenericAgent> agents = agent_grid(*geo);
    agents.emplace_back(make_agent(*geo, Point{0.0, 0.0}, Vector{-1.0, 0.0}));

    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    neighborhood_search.update(agents);
    OperationalDecisionSystem system{std::make_unique<FailingModel>()};
    JobQueue jobs{3};
    EXPECT_THROW(system.run(0.1, 0.0, neighborhood_search, *geo, agents, jobs), SimulationError);
}

TEST(OperationalDecisionSystemParallel, ModelsWithoutParallelSupportStayOnTheCallingThread)
{
    auto geo = flat_square();
    auto agents = agent_grid(*geo);

    NeighborhoodSearch<GenericAgent> neighborhood_search{2.2};
    neighborhood_search.update(agents);
    std::atomic<bool> left_creator{false};
    OperationalDecisionSystem system{std::make_unique<ThreadRecordingModel>(left_creator)};
    JobQueue jobs{3};
    system.run(0.1, 0.0, neighborhood_search, *geo, agents, jobs);

    EXPECT_FALSE(left_creator);
}
