// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgentState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"

#include <fmt/core.h>

class SocialForceModel : public OperationalModel
{
public:
    using State = SfmState;

private:
    double _cutOffRadius{2.5};
    double bodyForce{120000}; // k
    double friction{240000}; // kappa

public:
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

    SocialForceModel(double bodyForce, double friction);
    ~SocialForceModel() override = default;
    OperationalModelType Type() const override;

    void GetNeighbors(
        const GenericState& current,
        const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
        const CollisionGeometry& geometry,
        StateContainer& neighbor_states) const override;

    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const AgentJourney& journey,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const override;

    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;

private:
    static Point DrivingForce(const State& agent, const AgentJourney& journey);
    Point AgentForce(const State& ped1, const State& ped2) const;
    Point ObstacleForce(const State& agent, const LineSegment& segment) const;
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
