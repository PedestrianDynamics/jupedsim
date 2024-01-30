#include "SocialForceModel.hpp"

#include "Ellipse.hpp"
#include "SocialForceModelData.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"

#include <Logger.hpp>
#include <stdexcept>

SocialForceModel::SocialForceModel(double bodyForce_, double friction_):bodyForce(bodyForce_), friction(friction_){};

OperationalModelType SocialForceModel::Type() const
{
    return OperationalModelType::SOCIAL_FORCE;
}

std::unique_ptr<OperationalModel> SocialForceModel::Clone() const
{
    return std::make_unique<SocialForceModel>(*this);
}

OperationalModelUpdate SocialForceModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto& model = std::get<SocialForceModelData>(ped.model);
    SocialForceModelUpdate update{};

    update.desiredDirection = model.desiredDirection;
    update.position = ped.pos;
    update.velocity = model.velocity;
    return update;
}

void SocialForceModel::ApplyUpdate(
    const OperationalModelUpdate& update, 
    GenericAgent& agent) const 
{
    auto& model = std::get<SocialForceModelData>(agent.model);
    const auto& upd = std::get<SocialForceModelUpdate>(update);
    model.desiredDirection = upd.desiredDirection;
    agent.pos = upd.position;
    model.velocity = upd.velocity;
    agent.orientation = upd.velocity.Normalized();
}