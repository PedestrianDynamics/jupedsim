// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

enum class OperationalModelType {
    CollisionFreeSpeed,
    GeneralizedCentrifugalForce,
    CollisionFreeSpeedV2,
    CollisionFreeSpeedV3,
    AnticipationVelocityModel,
    SocialForce,
    WarpDriver,
    CustomModel
};

/// Names match the classes of the public Python API so that error messages are
/// actionable for API users.
constexpr const char* to_string(OperationalModelType type)
{
    switch(type) {
        case OperationalModelType::CollisionFreeSpeed:
            return "CollisionFreeSpeedModel";
        case OperationalModelType::GeneralizedCentrifugalForce:
            return "GeneralizedCentrifugalForceModel";
        case OperationalModelType::CollisionFreeSpeedV2:
            return "CollisionFreeSpeedModelV2";
        case OperationalModelType::CollisionFreeSpeedV3:
            return "CollisionFreeSpeedModelV3";
        case OperationalModelType::AnticipationVelocityModel:
            return "AnticipationVelocityModel";
        case OperationalModelType::SocialForce:
            return "SocialForceModel";
        case OperationalModelType::WarpDriver:
            return "WarpDriverModel";
        case OperationalModelType::CustomModel:
            return "CustomModel";
    }
    return "Unknown";
}
