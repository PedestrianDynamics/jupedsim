// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModelState.hpp"

class EnvironmentQuery;

#include <fmt/core.h>

class SocialForceModel : public OperationalModel
{
public:
    using State = SocialForceModelState;

private:
    double _cutOffRadius{2.5};
    double bodyForce{120000};
    double friction{240000};

public:
    SocialForceModel(double bodyForce, double friction);
    ~SocialForceModel() override = default;
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
    static Point DrivingForce(const State& state, const Point& destination);
    Point AgentForce(const State& s1, const State& s2) const;
    Point ObstacleForce(const State& state, const LineSegment& segment) const;
    static Point ForceBetweenPoints(
        const Point pt1,
        const Point pt2,
        const double A,
        const double B,
        const double radius,
        const Point velocity,
        const double bodyForce,
        const double friction);
    static double PushingForceLength(double A, double B, double r, double distance);
};
