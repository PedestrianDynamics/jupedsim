// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

enum class OperationalModelType {
    CollisionFreeSpeedTordeux2016,
    GeneralizedCentrifugalForce,
    CollisionFreeSpeed,
    MotivationCollisionFreeSpeed,
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
        case OperationalModelType::CollisionFreeSpeedTordeux2016:
            return "CollisionFreeSpeedModelTordeux2016";
        case OperationalModelType::GeneralizedCentrifugalForce:
            return "GeneralizedCentrifugalForceModel";
        case OperationalModelType::CollisionFreeSpeed:
            return "CollisionFreeSpeedModel";
        case OperationalModelType::MotivationCollisionFreeSpeed:
            return "MotivationCollisionFreeSpeedModel";
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
