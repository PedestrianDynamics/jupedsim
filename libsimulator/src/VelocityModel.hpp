// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"
#include "VelocityModelData.hpp"

struct GenericAgent;

class VelocityModel : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _cutOffRadius{3};
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;

public:
    VelocityModel(double aped, double Dped, double awall, double Dwall);
    ~VelocityModel() override = default;
    OperationalModelType Type() const override;
    OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const override;
    void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override;
    void CheckDistanceConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch) const override;
    std::unique_ptr<OperationalModel> Clone() const override;

private:
    double OptimalSpeed(const GenericAgent& ped, double spacing, double time_gap) const;
    double
    GetSpacing(const GenericAgent& ped1, const GenericAgent& ped2, const Point& direction) const;
    Point NeighborRepulsion(const GenericAgent& ped1, const GenericAgent& ped2) const;
    Point BoundaryRepulsion(const GenericAgent& ped, const LineSegment& boundary_segment) const;
};
