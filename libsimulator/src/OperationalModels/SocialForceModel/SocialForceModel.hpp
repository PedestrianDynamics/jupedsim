// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModelState.hpp"

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
    void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        Point destination,
        const CollisionGeometry& geometry,
        const NeighborQuery& neighborQuery) const override;
    void CheckModelConstraint(
        const OperationalModelState& state,
        const NeighborQuery& neighborQuery,
        const CollisionGeometry& geometry) const override;

private:
    /**
     * Driving force acting on pedestrian <model>
     * @param model state of the Pedestrian
     * @param destination the pedestrian's current routing waypoint
     *
     * @return vector with driving force of pedestrian
     */
    static Point DrivingForce(const State& model, Point destination);
    /**
     *  Repulsive force acting on pedestrian <model1> from pedestrian <model2>
     * @param model1 state of Pedestrian 1 on whom the force acts on
     * @param model2 state of Pedestrian 2, from whom the force originates
     * @return vector with the repulsive force
     */
    Point AgentForce(const State& model1, const State& model2) const;
    /**
     *  Repulsive force acting on pedestrian <model> from line segment <segment>
     * @param model state of the Pedestrian on whom the force acts on
     * @param segment reference to line segment, from which the force originates
     * @return vector with the repulsive force
     */
    Point ObstacleForce(const State& model, const LineSegment& segment) const;
    /**
     * calculates the pushing and friction forces acting between <pt1> and <pt2>
     * @param pt1 Point on which the forces act
     * @param pt2 Point from which the forces originate
     * @param A State scale
     * @param B force distance
     * @param r radius
     * @param velocity velocity difference
     * @param bodyForce body force parameter (k) of the agent the force acts on
     * @param friction friction parameter (kappa) of the agent the force acts on
     */
    static Point ForceBetweenPoints(
        const Point pt1,
        const Point pt2,
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
