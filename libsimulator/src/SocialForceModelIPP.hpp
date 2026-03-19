// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "SocialForceModelIPPData.hpp"
#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

struct GenericAgent;

class SocialForceModelIPP : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _cutOffRadius{2.5};

public:
    // Anthropometric scaling factor for ground support radius
    // foot length / (diameter * reference height)
    static constexpr double GS_SCALING_FACTOR =
        0.26 / (2 * 0.3 * 1.65);

    SocialForceModelIPP();
    ~SocialForceModelIPP() override = default;
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

private:
    /**
     * Driving force: (v0 * e0 - v) / tau
     */
    static Point DrivingForce(const GenericAgent& agent);

    /**
     * Exponential repulsion between two points: A * exp(-|x1-x2| / B) * n_hat
     * @param pt1 Point on which the force acts
     * @param pt2 Point from which the force originates
     * @param A repulsion amplitude
     * @param B interaction range (decay length)
     */
    static Point ExponentialRepulsion(
        const Point pt1,
        const Point pt2,
        double A,
        double B);
};
