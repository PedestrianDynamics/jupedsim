// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "LineSegment.hpp"

class EnvironmentQuery;
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

struct NeighborView;

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    /// Per-agent state of the generalized centrifugal force model.
    struct State {
        Point orientation{1.0, 0.0};
        double speed{};
        Point e0{};
        int orientationDelay{};
        double mass{1.0};
        double tau{0.5};
        double v0{1.2};
        double Av{1.0};
        double AMin{0.2};
        double BMin{0.2};
        double BMax{0.4};
    };

private:
    double _cutOffRadius{4.0}; // TODO (MC) check this free parameter
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
    Point ComputeNextState(
        const OperationalModelState& current,
        OperationalModelState& next,
        const AgentStep& step) const override;
    void CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const override;

private:
    /**
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param self State of the pedestrian the force acts on
     * @param to_target Vector from the pedestrian to its next target
     *
     * @return Point
     */
    Point ForceDriv(
        const State& self,
        Point to_target,
        double mass,
        double tau,
        double deltaT,
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
    Point ForceRepPed(const State& self, const NeighborView& neighbor) const;
    /**
     * Sum of the repulsive forces of all walls surrounding the pedestrian.
     * @see ForceRepWall
     */
    Point ForceRepRoom(const State& self, const AgentStep& step) const;
    Point ForceRepWall(const State& self, const LineSegment& w) const;
    Point ForceRepStatPoint(const State& self, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const State& self, const NeighborView& neighbor) const;
};

template <>
struct fmt::formatter<GeneralizedCentrifugalForceModel::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GeneralizedCentrifugalForceModel::State& m, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "GCFM[orientation={}, speed={}])", m.orientation, m.speed);
    }
};
