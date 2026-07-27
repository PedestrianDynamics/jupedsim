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

using OperationalModelState = std::variant<
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
    CustomModelState>;

inline const Point& Position(const OperationalModelState& state)
{
    return std::visit([](const auto& s) -> const Point& { return s.position; }, state);
}

inline Point& Position(OperationalModelState& state)
{
    return std::visit([](auto& s) -> Point& { return s.position; }, state);
}