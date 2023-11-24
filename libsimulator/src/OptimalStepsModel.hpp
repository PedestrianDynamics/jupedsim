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

    int numberCircles{};
    int positionsPerCircle{};

    double intimateWidth{}; // \delta_{i}
    double personalWidth{}; // \delta_{per}
    double repulsionIntensity{}; // \mu_p

    double geometryWidth{};
    double geometryHeight{};

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
    double computeDistancePotential(const Point& position, const Point& destination);
    double computeNeighborsPotential(
        const Point& position,
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch);
    double computeNeighborPotential(
        const Point& position,
        const GenericAgent& agent,
        const GenericAgent& otherAgent);

    double computeBoundaryPotential(
        const Point& position,
        const GenericAgent& agent,
        const CollisionGeometry& geometry);
};
