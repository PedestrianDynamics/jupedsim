// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

#include <memory>

class SocialForceModelBuilder
{
    double _bodyForce;
    double _friction;

public:
    SocialForceModelBuilder(double bodyForce, double friction);
    std::unique_ptr<OperationalModel> Build();
};
