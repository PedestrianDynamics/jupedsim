// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModelBuilder.hpp"
#include "SocialForceModel.hpp"

SocialForceModelBuilder::SocialForceModelBuilder(double bodyForce, double friction)
    : _bodyForce(bodyForce), _friction(friction)
{
}

SocialForceModel SocialForceModelBuilder::Build()
{
    return SocialForceModel(_bodyForce, _friction);
}
