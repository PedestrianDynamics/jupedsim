// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "SocialForceModel.hpp"
class SocialForceModelBuilder
{
    double _bodyForce;
    double _friction;

public:
    SocialForceModelBuilder(double bodyForce, double friction);
    SocialForceModel Build();
};
