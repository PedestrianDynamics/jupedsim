// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "CollisionGeometry.hpp"
#include "GeneralizedCentrifugalForceModelState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"

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
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param model state of the Pedestrian
     * @param target the pedestrian's current routing waypoint
     *
     * @return Point
     */
    Point ForceDriv(
        const State& model,
        Point target,
        double mass,
        double tau,
        double deltaT,
        Point& e0update) const;
    /**
     * Repulsive force between two pedestrians model1 and model2 according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param model1 state of the first pedestrian
     * @param model2 state of the second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const State& model1, const State& model2) const;
    /**
     * Sum of the repulsive forces acting on pedestrian <model> from all wall
     * segments of the collision geometry near the pedestrian.
     * @see ForceRepWall
     * @param model state of the Pedestrian
     * @param geometry collision geometry providing the walls
     *
     * @return
     */
    Point ForceRepRoom(const State& model, const CollisionGeometry& geometry) const;
    Point ForceRepWall(const State& model, const LineSegment& l) const;
    Point ForceRepStatPoint(const State& model, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const State& model1, const State& model2) const;
};
