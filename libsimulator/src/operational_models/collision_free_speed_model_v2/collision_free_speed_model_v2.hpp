// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "collision_free_speed_model_v2_state.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class CollisionFreeSpeedModelV2 : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelV2State;

private:
    double _cut_off_radius{3};

public:
    CollisionFreeSpeedModelV2() = default;
    ~CollisionFreeSpeedModelV2() override = default;
    OperationalModelType type() const override;
    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    double optimal_speed(const State& current_state, double spacing, double time_gap) const;
    double get_spacing(
        const State& current_state,
        const NeighborView& neighbor,
        const Point& direction) const;
    Point neighbor_repulsion(const State& current_state, const NeighborView& neighbor) const;
    Point boundary_repulsion(const State& current_state, const WallView& boundary) const;
};
