// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelV2Builder.hpp"

#include "CollisionFreeSpeedModelV2.hpp"

CollisionFreeSpeedModelV2Builder::CollisionFreeSpeedModelV2Builder()
{
}

std::unique_ptr<OperationalModel> CollisionFreeSpeedModelV2Builder::Build()
{
    return std::make_unique<CollisionFreeSpeedModelV2>();
}
