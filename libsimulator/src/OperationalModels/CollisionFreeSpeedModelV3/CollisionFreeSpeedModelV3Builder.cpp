// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV3Builder.hpp"

#include "CollisionFreeSpeedModelV3.hpp"

CollisionFreeSpeedModelV3Builder::CollisionFreeSpeedModelV3Builder()
{
}

std::unique_ptr<OperationalModel> CollisionFreeSpeedModelV3Builder::Build()
{
    return std::make_unique<CollisionFreeSpeedModelV3>();
}
