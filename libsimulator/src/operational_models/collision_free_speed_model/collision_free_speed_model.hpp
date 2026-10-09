// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "collision_free_speed_model_state.hpp"
#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class CollisionFreeSpeedModel : public OperationalModel
{
public:
    using State = CollisionFreeSpeedModelState;

private:
    double _cut_off_radius{3};
    double _strength_neighbor_repulsion{8.0};
    double _range_neighbor_repulsion{0.1};
    double _strength_geometry_repulsion{5.0};
    double _range_geometry_repulsion{0.02};

public:
    CollisionFreeSpeedModel(
        double strength_neighbor_repulsion,
        double range_neighbor_repulsion,
        double strength_geometry_repulsion,
        double range_geometry_repulsion);
    ~CollisionFreeSpeedModel() override = default;
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
