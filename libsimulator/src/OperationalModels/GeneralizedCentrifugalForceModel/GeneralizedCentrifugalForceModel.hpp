// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "LineSegment.hpp"

class EnvironmentQuery;
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    /// Per-agent state of the generalized centrifugal force model.
    struct State {
        Point position{};
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
    void ComputeNextState(
        double dT,
        const GenericAgent& current,
        GenericAgent& next,
        const EnvironmentQuery& envQuery) const override;
    void CheckModelConstraint(const GenericAgent& agent, const EnvironmentQuery& envQuery)
        const override;

private:
    /**
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param ped Pointer to Pedestrians
     * @param room Pointer to Room
     *
     * @return Point
     */
    Point ForceDriv(
        const GenericAgent& ped,
        Point target,
        double mass,
        double tau,
        double deltaT,
        Point& e0update) const;
    /**
     * Repulsive force between two pedestrians ped1 and ped2 according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param ped1 Pointer to Pedestrian: First pedestrian
     * @param ped2 Pointer to Pedestrian: Second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const GenericAgent& ped1, const GenericAgent& ped2) const;
    /**
     * Repulsive force acting on pedestrian <ped> from the walls in
     * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
     * @see ForceRepWall
     * @param ped Pointer to Pedestrian
     * @param subroom Pointer to SubRoom
     *
     * @return
     */
    Point ForceRepRoom(const GenericAgent& ped, const EnvironmentQuery& envQuery) const;
    Point ForceRepWall(const GenericAgent& ped, const LineSegment& l) const;
    Point ForceRepStatPoint(const GenericAgent& ped, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const GenericAgent& agent, const GenericAgent& otherAgent) const;
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
