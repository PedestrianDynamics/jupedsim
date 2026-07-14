// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "AgentJourney.hpp"
#include "CollisionGeometry.hpp"
#include "GenericAgentState.hpp"
#include "LineSegment.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"

#include <fmt/core.h>

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    using State = GcfmState;

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
    using OperationalModel::GenericState;
    using OperationalModel::StateContainer;

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
    /**
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param state Pointer to Pedestrians
     * @param room Pointer to Room
     *
     * @return Point
     */
    Point ForceDriv(
        const State& state,
        Point target,
        double mass,
        double tau,
        double deltaT,
        Point& e0update) const;
    /**
     * Repulsive force between two pedestrians state1 and state2 according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param state1 Pointer to Pedestrian: First pedestrian
     * @param state2 Pointer to Pedestrian: Second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const State& state1, const State& state2) const;
    /**
     * Repulsive force acting on pedestrian <state> from the walls in
     * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
     * @see ForceRepWall
     * @param state Pointer to Pedestrian
     * @param subroom Pointer to SubRoom
     *
     * @return
     */
    Point ForceRepRoom(const State& state, const CollisionGeometry& geometry) const;
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
    double AgentToAgentSpacing(const State& state, const State& otherstate) const;
};
