// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <unordered_map>
#include <vector>

struct GenericAgent;

class GeneralizedCentrifugalForceModel : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _nuPed; /**< strength of the pedestrian repulsive force */
    double _nuWall; /**< strength of the wall repulsive force */
    double _intp_widthPed; /**< Interpolation cutoff radius (in cm) */
    double _intp_widthWall; /**< Interpolation cutoff radius (in cm) */
    double _maxfPed;
    double _maxfWall;
    double _distEffMaxPed; // maximal effective distance
    double _distEffMaxWall; // maximal effective distance
public:
    GeneralizedCentrifugalForceModel(
        double nuped,
        double nuwall,
        double dist_effPed,
        double dist_effWall,
        double intp_widthped,
        double intp_widthwall,
        double maxfped,
        double maxfwall);
    ~GeneralizedCentrifugalForceModel() override = default;

    OperationalModelType Type() const override;
    OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& agent,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const override;
    void ApplyUpdate(const OperationalModelUpdate& upate, GenericAgent& agent) const override;
    void CheckDistanceConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch) const override;
    std::unique_ptr<OperationalModel> Clone() const override;

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
        GeneralizedCentrifugalForceModelUpdate& update) const;
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
    Point ForceRepRoom(const GenericAgent& ped, const CollisionGeometry& geometry) const;
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
