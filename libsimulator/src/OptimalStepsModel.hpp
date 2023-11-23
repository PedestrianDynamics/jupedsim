// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OptimalStepsModelData.hpp"
#include "RoutingEngine.hpp"
#include "UniqueID.hpp"

struct GenericAgent;

class OptimalStepsModel : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _cutOffRadius{3};
    double strengthNeighborRepulsion;
    double rangeNeighborRepulsion;
    double strengthGeometryRepulsion;
    double rangeGeometryRepulsion;
    RoutingEngine* routingEngine{};

public:
    OptimalStepsModel(
        double strengthNeighborRepulsion,
        double rangeNeighborRepulsion,
        double strengthGeometryRepulsion,
        double rangeGeometryRepulsion);
    ~OptimalStepsModel() override = default;
    OperationalModelType Type() const override;
    OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const override;
    void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override;
    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;
    std::unique_ptr<OperationalModel> Clone() const override;
    void SetRoutingEngine(RoutingEngine* routingEngine_) { routingEngine = routingEngine_; }

private:
    double OptimalSpeed(const GenericAgent& ped, double spacing, double time_gap) const;
    double
    GetSpacing(const GenericAgent& ped1, const GenericAgent& ped2, const Point& direction) const;
    Point NeighborRepulsion(const GenericAgent& ped1, const GenericAgent& ped2) const;
    Point BoundaryRepulsion(const GenericAgent& ped, const LineSegment& boundary_segment) const;

    double computeDistancePotential(const Point& position, const Point& destination);
    double computeNeighborPotential(
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch);
    double computeBoundaryPotential(const Point& position, const CollisionGeometry& geometry);
};
