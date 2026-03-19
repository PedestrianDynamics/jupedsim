// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "SocialForceModelIPP.hpp"
class SocialForceModelIPPBuilder
{
    double _bodyForce;
    double _friction;

public:
    SocialForceModelIPPBuilder(double bodyForce, double friction);
    SocialForceModelIPP Build();
};
