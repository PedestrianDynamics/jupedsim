// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "LineSegment.hpp"

class EnvironmentQuery;
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

class SocialForceModel : public OperationalModel
{
public:
    /// Per-agent state of the social force model.
    struct State {
        Point position{};
        Point velocity{}; // v
        double mass{80.0}; // m
        double desiredSpeed{0.8}; // v0
        double reactionTime{0.5}; // tau
        double agentScale{2000.0}; // A for other agents
        double obstacleScale{2000.0}; // A for obstacles
        double forceDistance{0.08}; // B
        double radius{0.3}; // r
    };

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
        const GenericAgent& current,
        GenericAgent& next,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(
        const GenericAgent& agent,
        const EnvironmentQuery& envQuery) const override;

private:
    /**
     * Driving force acting on pedestrian <agent>
     * @param agent reference to Pedestrian
     *
     * @return vector with driving force of pedestrian
     */
    static Point DrivingForce(const GenericAgent& agent);
    /**
     *  Repulsive force acting on pedestrian <ped1> from pedestrian <ped2>
     * @param ped1 reference to Pedestrian 1 on whom the force acts on
     * @param ped2 reference to Pedestrian 2, from whom the force originates
     * @return vector with the repulsive force
     */
    Point AgentForce(const GenericAgent& ped1, const GenericAgent& ped2) const;
    /**
     *  Repulsive force acting on pedestrian <agent> from line segment <segment>
     * @param agent reference to the Pedestrian on whom the force acts on
     * @param segment reference to line segment, from which the force originates
     * @return vector with the repulsive force
     */
    Point ObstacleForce(const GenericAgent& agent, const LineSegment& segment) const;
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

template <>
struct fmt::formatter<SocialForceModel::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const SocialForceModel::State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "SFM[velocity={}, m={}, v0={}, tau={}, A_ped={}, A_obst={}, B={}, r={}])",
            m.velocity,
            m.mass,
            m.desiredSpeed,
            m.reactionTime,
            m.agentScale,
            m.obstacleScale,
            m.forceDistance,
            m.radius);
    }
};
