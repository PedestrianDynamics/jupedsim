// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

#include <memory>

class CollisionFreeSpeedModelV3Builder
{
public:
    CollisionFreeSpeedModelV3Builder();
    std::unique_ptr<OperationalModel> Build();
};
