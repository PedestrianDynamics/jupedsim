// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModelState.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class SocialForceModel : public OperationalModel
{
public:
    using State = SocialForceModelState;

private:
    double _cutOffRadius{2.5};
    double bodyForce{120000}; // k
    double friction{240000}; // kappa

public:
    SocialForceModel(double bodyForce, double friction);
    ~SocialForceModel() override = default;
    OperationalModelType Type() const override;
    Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    /**
     * Driving force acting on pedestrian <agent>
     * @param agent reference to Pedestrian
     *
     * @return vector with driving force of pedestrian
     */
    static Point DrivingForce(const State& currentState, Point ToNextTarget);
    /**
     *  Repulsive force acting on pedestrian <ped1> from pedestrian <ped2>
     * @param ped1 reference to Pedestrian 1 on whom the force acts on
     * @param ped2 reference to Pedestrian 2, from whom the force originates
     * @return vector with the repulsive force
     */
    Point AgentForce(const State& currentState, const NeighborView& neighbor) const;
    /**
     *  Repulsive force acting on pedestrian <agent> from line segment <segment>
     * @param agent reference to the Pedestrian on whom the force acts on
     * @param segment reference to line segment, from which the force originates
     * @return vector with the repulsive force
     */
    Point ObstacleForce(const State& currentState, const WallView& wall) const;
    /**
     * calculates the pushing and friction forces along <separation>
     * @param separation vector pointing from where the force originates to where it acts
     * @param A State scale
     * @param B force distance
     * @param r radius
     * @param velocity velocity difference
     * @param bodyForce body force parameter (k) of the agent the force acts on
     * @param friction friction parameter (kappa) of the agent the force acts on
     */
    static Point ForceFromSeparation(
        const Point separation,
        const double A,
        const double B,
        const double radius,
        const Point velocity,
        const double bodyForce,
        const double friction);

    /**
     *  exponential function that specifies the length of the pushing force between two points
     * @param A State scale
     * @param B force distance
     * @param r radius
     * @param distance distance between the two points
     * @return length of pushing force between the two points
     */
    static double PushingForceLength(double A, double B, double r, double distance);
};
