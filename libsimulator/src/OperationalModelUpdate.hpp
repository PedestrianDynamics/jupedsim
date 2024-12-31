// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelUpdate.hpp"
#include "CollisionFreeSpeedModelV2Update.hpp"
#include "CollisionFreeSpeedModelV3Update.hpp"
#include "GeneralizedCentrifugalForceModelUpdate.hpp"
#include "SocialForceModelUpdate.hpp"

#include <variant>

using OperationalModelUpdate = std::variant<
    GeneralizedCentrifugalForceModelUpdate,
    CollisionFreeSpeedModelUpdate,
    CollisionFreeSpeedModelV2Update,
    CollisionFreeSpeedModelV3Update,
    SocialForceModelUpdate>;
