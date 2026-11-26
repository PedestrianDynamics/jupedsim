// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "anticipation_velocity_model_state.hpp"
#include "collision_free_speed_model_state.hpp"
#include "collision_free_speed_model_tordeux2016_state.hpp"
#include "custom_model_state.hpp"
#include "generalized_centrifugal_force_model_state.hpp"
#include "motivation_collision_free_speed_model_state.hpp"
#include "social_force_model_state.hpp"
#include "warp_driver_model_state.hpp"

#include <variant>

using OperationalModelState = std::variant<
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelTordeux2016State,
    CollisionFreeSpeedModelState,
    MotivationCollisionFreeSpeedModelState,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
    CustomModelState>;
