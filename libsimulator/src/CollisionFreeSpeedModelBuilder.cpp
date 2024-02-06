// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelBuilder.hpp"

CollisionFreeSpeedModelBuilder::CollisionFreeSpeedModelBuilder(
    double aPed,
    double DPed,
    double aWall,
    double DWall)
    : _aPed(aPed), _DPed(DPed), _aWall(aWall), _DWall(DWall)
{
}

CollisionFreeSpeedModel CollisionFreeSpeedModelBuilder::Build()
{
    return CollisionFreeSpeedModel(_aPed, _DPed, _aWall, _DWall);
}
