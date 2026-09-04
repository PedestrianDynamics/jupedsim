// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GeneralizedCentrifugalForceModelState.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

#include <fmt/core.h>

struct NeighborView;
struct WallView;

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    using State = GeneralizedCentrifugalForceModelState;

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
        const State& currentState,
        Point orientationToTarget,
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
    Point ForceRepPed(const State& currentState, const NeighborView& neighbor) const;
    /**
     * Sum of the repulsive forces of all walls surrounding the pedestrian.
     * @see ForceRepWall
     */
    Point ForceRepWall(const State& currentState, const WallView& wall) const;
    Point ForceRepStatPoint(const State& currentState, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const State& currentState, const NeighborView& neighbor) const;
};
