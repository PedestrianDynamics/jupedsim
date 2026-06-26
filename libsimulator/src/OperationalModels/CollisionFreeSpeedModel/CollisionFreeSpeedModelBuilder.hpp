// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

#include <memory>

class CollisionFreeSpeedModelBuilder
{
    double _aPed;
    double _DPed;
    double _aWall;
    double _DWall;

public:
    CollisionFreeSpeedModelBuilder(double aPed, double DPed, double aWall, double DWall);
    std::unique_ptr<OperationalModel> Build();
};
