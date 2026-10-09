// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <vector>

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
    // Shared sequential RNG: draws must stay on the model to keep simulations deterministic.
    mutable std::mt19937 _gen;

public:
    AnticipationVelocityModel(double pushout_strength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
    OperationalModelType type() const override;
    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    double optimal_speed(const State& current_state, double spacing, double time_gap) const;
    Point calculate_influence_direction(
        const Point& desired_direction,
        const Point& predicted_direction) const;
    double get_spacing(
        const State& current_state,
        const NeighborView& neighbor,
        const Point& direction) const;
    Point neighbor_repulsion(
        const State& current_state,
        Point desired_direction,
        const NeighborView& neighbor) const;

    Point handle_wall_avoidance(
        const Point& direction,
        const State& current_state,
        const AgentStep& step,
        double pushout_strength) const;

    Point update_direction(
        const State& current_state,
        Point desired_direction,
        const Point& calculated_direction,
        double dt) const;
};
