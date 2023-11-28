// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OptimalStepsModelData.hpp"
#include "RoutingEngine.hpp"
#include "UniqueID.hpp"

#include "RNG.hpp"
#include <optional>

struct GenericAgent;

class OptimalStepsModel : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _cutOffRadius{3};

    int numberCircles{1};
    int positionsPerCircle{4};

    static constexpr double stepLengthIntercept{0.4625};
    static constexpr double stepLengthSlopeSpeed{0.2345};
    double stepLengthSD{0.036};
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
    double computeDistancePotential(const Point& position, const Point& destination) const;
    double computeNeighborsPotential(
        const Point& position,
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch) const;
    double computeNeighborPotential(
        const Point& position,
        const GenericAgent& agent,
        const GenericAgent& otherAgent) const;

    double computeBoundaryPotential(
        const Point& position,
        const GenericAgent& agent,
        const CollisionGeometry& geometry) const;

    std::optional<Point> computeCandiateOnCircle(
        const Point& center,
        double radius,
        int count,
        int numberOfPoints,
        const std::vector<LineSegment>& walls) const;

    double computePotential(
        const Point& position,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const;

    double computeDesiredStepSize(const GenericAgent& agent) const;
    double computeNextTimeToAct(const GenericAgent& agent, double stepSize) const;
};
