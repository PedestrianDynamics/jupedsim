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

SocialForceModel::SocialForceModel(double test_value_):test_value(test_value_){};

OperationalModelType SocialForceModel::Type() const
{
    return OperationalModelType::SOCIAL_FORCE;
}

std::unique_ptr<OperationalModel> SocialForceModel::Clone() const
{
    return std::make_unique<SocialForceModel>(*this);
}