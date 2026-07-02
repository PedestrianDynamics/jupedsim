// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

enum class OperationalModelType {
    COLLISION_FREE_SPEED,
    GENERALIZED_CENTRIFUGAL_FORCE,
    COLLISION_FREE_SPEED_V2,
    COLLISION_FREE_SPEED_V3,
    ANTICIPATION_VELOCITY_MODEL,
    SOCIAL_FORCE,
    WARP_DRIVER,
    CUSTOM_MODEL
};

/// Names match the classes of the public Python API so that error messages are
/// actionable for API users.
constexpr const char* ToString(OperationalModelType type)
{
    switch(type) {
        case OperationalModelType::COLLISION_FREE_SPEED:
            return "CollisionFreeSpeedModel";
        case OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE:
            return "GeneralizedCentrifugalForceModel";
        case OperationalModelType::COLLISION_FREE_SPEED_V2:
            return "CollisionFreeSpeedModelV2";
        case OperationalModelType::COLLISION_FREE_SPEED_V3:
            return "CollisionFreeSpeedModelV3";
        case OperationalModelType::ANTICIPATION_VELOCITY_MODEL:
            return "AnticipationVelocityModel";
        case OperationalModelType::SOCIAL_FORCE:
            return "SocialForceModel";
        case OperationalModelType::WARP_DRIVER:
            return "WarpDriverModel";
        case OperationalModelType::CUSTOM_MODEL:
            return "CustomModel";
    }
    return "Unknown";
}
