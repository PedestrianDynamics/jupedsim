// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GeneralizedCentrifugalForceModelState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

class EnvironmentQuery;

#include <fmt/core.h>

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    using State = GeneralizedCentrifugalForceModelState;

private:
    double _cutOffRadius{4.0};
    double strengthNeighborRepulsion{0.3};
    double strengthGeometryRepulsion{0.2};
    double maxNeighborInteractionDistance{2};
    double maxGeometryInteractionDistance{2};
    double maxNeighborInterpolationDistance{0.1};
    double maxGeometryInterpolationDistance{0.1};
    double maxNeighborRepulsionForce{9};
    double maxGeometryRepulsionForce{3};

public:
    GeneralizedCentrifugalForceModel(
        double strengthNeighborRepulsion,
        double strengthGeometryRepulsion,
        double maxNeighborInteractionDistance,
        double maxGeometryInteractionDistance,
        double maxNeighborInterpolationDistance,
        double maxGeometryInterpolationDistance,
        double maxNeighborRepulsionForce,
        double maxGeometryRepulsionForce);
    ~GeneralizedCentrifugalForceModel() override = default;

    OperationalModelType Type() const override;
    void ComputeNextState(
        double dT,
        const OperationalModelState& current,
        OperationalModelState& next,
        const Point& destination,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(const OperationalModelState& state, const EnvironmentQuery& envQuery)
        const override;

private:
    Point ForceDriv(
        const State& state,
        Point target,
        double mass,
        double tau,
        double deltaT,
        Point& e0update) const;
    Point ForceRepPed(const State& s1, const State& s2) const;
    Point ForceRepRoom(const State& state, const EnvironmentQuery& envQuery) const;
    Point ForceRepWall(const State& state, const LineSegment& l) const;
    Point ForceRepStatPoint(const State& state, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const State& s1, const State& s2) const;
};
