// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionFreeSpeedModelUpdate.hpp"
#include "GeneralizedCentrifugalForceModelUpdate.hpp"

#include <variant>

using OperationalModelUpdate =
    std::variant<GeneralizedCentrifugalForceModelUpdate, CollisionFreeSpeedModelUpdate>;
