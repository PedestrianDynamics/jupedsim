// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GeneralizedCentrifugalForceModelState.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    using State = GeneralizedCentrifugalForceModelState;

private:
    double _cut_off_radius{4.0}; // TODO (MC) check this free parameter
    double _strength_neighbor_repulsion{0.3};
    double _strength_geometry_repulsion{0.2};
    double _max_neighbor_interaction_distance{2};
    double _max_geometry_interaction_distance{2};
    double _max_neighbor_interpolation_distance{0.1};
    double _max_geometry_interpolation_distance{0.1};
    double _max_neighbor_repulsion_force{9};
    double _max_geometry_repulsion_force{3};

public:
    GeneralizedCentrifugalForceModel(
        double strength_neighbor_repulsion,
        double strength_geometry_repulsion,
        double max_neighbor_interaction_distance,
        double max_geometry_interaction_distance,
        double max_neighbor_interpolation_distance,
        double max_geometry_interpolation_distance,
        double max_neighbor_repulsion_force,
        double max_geometry_repulsion_force);
    ~GeneralizedCentrifugalForceModel() override = default;

    OperationalModelType type() const override;
    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    /**
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param self State of the pedestrian the force acts on
     * @param to_target Vector from the pedestrian to its next target
     *
     * @return Point
     */
    Point force_driv(
        const State& current_state,
        Point orientation_to_target,
        double mass,
        double tau,
        double delta_t,
        Point& e0update) const;
    /**
     * Repulsive force between two pedestrians according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param self State of the pedestrian the force acts on
     * @param neighbor The other pedestrian, seen from the first one
     *
     * @return Point
     */
    Point force_rep_ped(const State& current_state, const NeighborView& neighbor) const;
    /**
     * Sum of the repulsive forces of all walls surrounding the pedestrian.
     * @see force_rep_wall
     */
    Point force_rep_wall(const State& current_state, const WallView& wall) const;
    Point
    force_rep_stat_point(const State& current_state, const Point& p, double l, double vn) const;
    Point force_interpolation(
        double v0,
        double k_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double agent_to_agent_spacing(const State& current_state, const NeighborView& neighbor) const;
};
