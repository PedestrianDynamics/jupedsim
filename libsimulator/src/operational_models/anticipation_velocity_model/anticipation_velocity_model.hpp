// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "agent_rng.hpp"
#include "anticipation_velocity_model_state.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "operational_model_type.hpp"

#include <fmt/core.h>

#include <cstdint>

struct NeighborView;
struct WallView;

class AnticipationVelocityModel : public OperationalModel
{
public:
    using State = AnticipationVelocityModelState;

private:
    /// Add a small outward component to maintain minimum distance from walls.
    double _pushout_strength{0.3};
    double _cut_off_radius{3};
    /// Hands out the agents' random number streams.
    AgentRngSeeder _rng_seeder;

public:
    AnticipationVelocityModel(double pushout_strength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
    OperationalModelType type() const override;
    Vector compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;
    void seed_agent_state(OperationalModelState& state) override;

private:
    double
    optimal_speed(const State& current_state, double spacing, double time_gap, AgentRng& rng) const;
    Vector calculate_influence_direction(
        const Vector& desired_direction,
        const Vector& predicted_direction,
        AgentRng& rng) const;
    double get_spacing(
        const State& current_state,
        const NeighborView& neighbor,
        const Vector& direction) const;
    Vector neighbor_repulsion(
        const State& current_state,
        Vector desired_direction,
        const NeighborView& neighbor,
        AgentRng& rng) const;

    Vector handle_wall_avoidance(
        const Vector& direction,
        const State& current_state,
        const AgentStep& step,
        double pushout_strength) const;

    Vector update_direction(
        const State& current_state,
        Vector desired_direction,
        const Vector& calculated_direction,
        double dt) const;
};
