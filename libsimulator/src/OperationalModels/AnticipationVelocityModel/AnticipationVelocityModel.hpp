// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "LineSegment.hpp"
class EnvironmentQuery;
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <random>
#include <vector>

class AnticipationVelocityModel : public OperationalModel
{
public:
    /// Per-agent state of the anticipation velocity model.
    struct State {
        Point position{};
        Point orientation{0.0, 0.0};
        double strengthNeighborRepulsion{8.0};
        double rangeNeighborRepulsion{0.1};
        double wallBufferDistance{0.1}; // buff distance of agent to wall
        double anticipationTime{1.0}; // anticipation time
        double reactionTime{0.3}; // reaction time to update direction
        Point velocity{};
        double timeGap{1.06};
        double v0{1.2};
        double radius{0.2};
    };

private:
    /// Add a small outward component to maintain minimum distance from walls.
    double _pushoutStrength{0.3};
    double _cutOffRadius{3};
    // Shared sequential RNG: draws must stay on the model to keep simulations deterministic.
    mutable std::mt19937 gen;

public:
    AnticipationVelocityModel(double pushoutStrength, uint64_t rng_seed);
    ~AnticipationVelocityModel() override = default;
    OperationalModelType Type() const override;
    void ComputeNextState(
        double dT,
        const GenericAgent& current,
        GenericAgent& next,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(const GenericAgent& agent, const EnvironmentQuery& envQuery)
        const override;

private:
    double OptimalSpeed(const GenericAgent& ped, double spacing, double time_gap) const;
    Point CalculateInfluenceDirection(
        const Point& desiredDirection,
        const Point& predictedDirection) const;
    double
    GetSpacing(const GenericAgent& ped1, const GenericAgent& ped2, const Point& direction) const;
    Point NeighborRepulsion(const GenericAgent& ped1, const GenericAgent& ped2) const;

    Point HandleWallAvoidance(
        const Point& direction,
        const Point& agentPosition,
        double agentRadius,
        const auto& boundary,
        double wallBufferDistance,
        double pushoutStrength) const;

    Point
    UpdateDirection(const GenericAgent& ped, const Point& calculatedDirection, double dt) const;
};

template <>
struct fmt::formatter<AnticipationVelocityModel::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const AnticipationVelocityModel::State& m, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "AnticipationVelocityModel[orientation={}, strengthNeighborRepulsion={}, "
            "rangeNeighborRepulsion={}, wallBufferDistance={}, "
            "timeGap={}, v0={}, radius={}, reactionTime={}, anticipationTime={}, velocity={}])",
            m.orientation,
            m.strengthNeighborRepulsion,
            m.rangeNeighborRepulsion,
            m.wallBufferDistance,
            m.timeGap,
            m.v0,
            m.radius,
            m.reactionTime,
            m.anticipationTime,
            m.velocity);
    }
};
