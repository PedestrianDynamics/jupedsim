#pragma once

#include "CollisionFreeSpeedModelData.hpp"
#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

struct GenericAgent;

class SocialForceModel : public OperationalModel
{
public:
    using NeighborhoodSearchType = NeighborhoodSearch<GenericAgent>;

private:
    double _cutOffRadius{2.5};
    double bodyForce;
    double friction;

public:
    SocialForceModel(double bodyForce_, double friction_);
    ~SocialForceModel() override = default;
    OperationalModelType Type() const override;
    OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const override{};
    void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override{};
    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearchType& neighborhoodSearch,
        const CollisionGeometry& geometry) const override{};
    std::unique_ptr<OperationalModel> Clone() const override;

private:
    
};
