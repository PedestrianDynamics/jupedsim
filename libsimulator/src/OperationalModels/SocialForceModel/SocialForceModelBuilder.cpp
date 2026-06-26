// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModelBuilder.hpp"

#include "SocialForceModel.hpp"

SocialForceModelBuilder::SocialForceModelBuilder(double bodyForce, double friction)
    : _bodyForce(bodyForce), _friction(friction)
{
}

std::unique_ptr<OperationalModel> SocialForceModelBuilder::Build()
{
    return std::make_unique<SocialForceModel>(_bodyForce, _friction);
}
