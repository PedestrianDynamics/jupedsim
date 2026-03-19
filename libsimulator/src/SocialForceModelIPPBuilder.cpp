// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModelIPPBuilder.hpp"
#include "SocialForceModelIPP.hpp"

SocialForceModelIPPBuilder::SocialForceModelIPPBuilder(double bodyForce, double friction)
    : _bodyForce(bodyForce), _friction(friction)
{
}

SocialForceModelIPP SocialForceModelIPPBuilder::Build()
{
    return SocialForceModelIPP(_bodyForce, _friction);
}
