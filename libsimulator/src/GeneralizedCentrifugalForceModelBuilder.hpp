// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GeneralizedCentrifugalForceModel.hpp"
#include <vector>

class GeneralizedCentrifugalForceModelBuilder
{
    double _nuped;
    double _nuwall;
    double _dist_effPed;
    double _dist_effWall;
    double _intp_widthped;
    double _intp_widthwall;
    double _maxfped;
    double _maxfwall;

public:
    GeneralizedCentrifugalForceModelBuilder(
        double nuped,
        double nuwall,
        double dist_effPed,
        double dist_effWall,
        double intp_widthped,
        double intp_widthwall,
        double maxfped,
        double maxfwall);
    GeneralizedCentrifugalForceModel Build();
};
