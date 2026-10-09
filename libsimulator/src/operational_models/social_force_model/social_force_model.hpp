// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "operational_model.hpp"
#include "operational_model_type.hpp"
#include "point.hpp"
#include "social_force_model_state.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class SocialForceModel : public OperationalModel
{
public:
    using State = SocialForceModelState;

private:
    double _cut_off_radius{2.5};
    double _body_force{120000}; // k
    double _friction{240000}; // kappa

public:
    SocialForceModel(double body_force, double friction);
    ~SocialForceModel() override = default;
    OperationalModelType type() const override;
    Point compute_next_state(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void check_model_constraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    /**
     * Driving force acting on pedestrian <agent>
     * @param agent reference to Pedestrian
     *
     * @return vector with driving force of pedestrian
     */
    static Point driving_force(const State& current_state, Point e0);
    /**
     *  Repulsive force acting on pedestrian <ped1> from pedestrian <ped2>
     * @param ped1 reference to Pedestrian 1 on whom the force acts on
     * @param ped2 reference to Pedestrian 2, from whom the force originates
     * @return vector with the repulsive force
     */
    Point agent_force(const State& current_state, const NeighborView& neighbor) const;
    /**
     *  Repulsive force acting on pedestrian <agent> from line segment <segment>
     * @param agent reference to the Pedestrian on whom the force acts on
     * @param segment reference to line segment, from which the force originates
     * @return vector with the repulsive force
     */
    Point obstacle_force(const State& current_state, const WallView& wall) const;
    /**
     * calculates the pushing and friction forces along <separation>
     * @param separation vector pointing from where the force originates to where it acts
     * @param A State scale
     * @param B force distance
     * @param r radius
     * @param velocity velocity difference
     * @param body_force body force parameter (k) of the agent the force acts on
     * @param friction friction parameter (kappa) of the agent the force acts on
     */
    static Point force_from_separation(
        const Point separation,
        const double a,
        const double b,
        const double radius,
        const Point velocity,
        const double body_force,
        const double friction);

    /**
     *  exponential function that specifies the length of the pushing force between two points
     * @param A State scale
     * @param B force distance
     * @param r radius
     * @param distance distance between the two points
     * @return length of pushing force between the two points
     */
    static double pushing_force_length(double a, double b, double r, double distance);
};
