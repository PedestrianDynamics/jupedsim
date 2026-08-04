// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "CollisionFreeSpeedModelState.hpp"
#include "CollisionFreeSpeedModelV2State.hpp"
#include "CollisionFreeSpeedModelV3State.hpp"
#include "CustomModelState.hpp"
#include "GeneralizedCentrifugalForceModelState.hpp"
#include "SocialForceModelState.hpp"
#include "WarpDriverModelState.hpp"

#include <variant>

using OperationalModelState = std::variant<
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
    CustomModelState>;
