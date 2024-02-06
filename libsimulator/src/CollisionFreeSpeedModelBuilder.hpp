// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionFreeSpeedModel.hpp"
class CollisionFreeSpeedModelBuilder
{
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;

public:
    CollisionFreeSpeedModelBuilder(double aPed, double DPed, double aWall, double DWall);
    CollisionFreeSpeedModel Build();
};
