// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

#include <memory>

class CollisionFreeSpeedModelV2Builder
{
public:
    CollisionFreeSpeedModelV2Builder();
    std::unique_ptr<OperationalModel> Build();
};
