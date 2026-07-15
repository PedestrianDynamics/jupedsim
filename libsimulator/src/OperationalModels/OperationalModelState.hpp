// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModel/AnticipationVelocityModelState.hpp"
#include "CollisionFreeSpeedModel/CollisionFreeSpeedModelState.hpp"
#include "CollisionFreeSpeedModelV2/CollisionFreeSpeedModelV2State.hpp"
#include "CollisionFreeSpeedModelV3/CollisionFreeSpeedModelV3State.hpp"
#include "CustomModel/CustomModelState.hpp"
#include "GeneralizedCentrifugalForceModel/GeneralizedCentrifugalForceModelState.hpp"
#include "SocialForceModel/SocialForceModelState.hpp"
#include "WarpDriver/WarpDriverModelState.hpp"

#include <variant>
// The variant that represents the per-agent operational model state.
using OperationalModelState = std::variant<
    AnticipationVelocityModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    CustomModelState,
    GeneralizedCentrifugalForceModelState,
    SocialForceModelState,
    WarpDriverModelState>;
