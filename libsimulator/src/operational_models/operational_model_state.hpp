// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "anticipation_velocity_model_state.hpp"
#include "collision_free_speed_model_state.hpp"
#include "collision_free_speed_model_v2_state.hpp"
#include "collision_free_speed_model_v3_state.hpp"
#include "custom_model_state.hpp"
#include "generalized_centrifugal_force_model_state.hpp"
#include "social_force_model_state.hpp"
#include "warp_driver_model_state.hpp"

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
