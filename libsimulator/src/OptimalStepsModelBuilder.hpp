// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OptimalStepsModel.hpp"
class OptimalStepsModelBuilder
{
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;

public:
    OptimalStepsModelBuilder(double aPed, double DPed, double aWall, double DWall);
    OptimalStepsModel Build();
};
