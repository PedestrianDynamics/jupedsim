// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "VelocityModel.hpp"
class VelocityModelBuilder
{
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;

public:
    VelocityModelBuilder(double aPed, double DPed, double aWall, double DWall);
    VelocityModel Build();
};
