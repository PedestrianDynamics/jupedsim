// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModelState.hpp"
#include "CollisionFreeSpeedModelState.hpp"
#include "CollisionFreeSpeedModelV2State.hpp"
#include "CollisionFreeSpeedModelV3State.hpp"
#include "CustomModel/CustomModelState.hpp"
#include "GeneralizedCentrifugalForceModelState.hpp"
#include "OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModelState.hpp"
#include "Visitor.hpp"
#include "WarpDriver/WarpDriverModelState.hpp"

#include <concepts>
#include <variant>

/// Agent position is owned by the per-model agent state. Every alternative of
/// OperationalModelState must satisfy this concept; the framework accesses the
/// position type-erased through Pos().
template <typename T>
concept ModelAgentState = requires(T t) {
    // Pos() hands out mutable Point& into the state, so a convertible or const member
    // is not enough.
    { t.position } -> std::same_as<Point&>;
};

template <typename Variant>
inline constexpr bool EachAlternativeIsModelAgentState = false;
template <typename... Ts>
inline constexpr bool EachAlternativeIsModelAgentState<std::variant<Ts...>> =
    (ModelAgentState<Ts> && ...);

/// Per-agent state of the operational model, one alternative per built-in model plus the
/// custom-model escape hatch.
using OperationalModelState = std::variant<
    GeneralizedCentrifugalForceModelState,
    CollisionFreeSpeedModelState,
    CollisionFreeSpeedModelV2State,
    CollisionFreeSpeedModelV3State,
    AnticipationVelocityModelState,
    SocialForceModelState,
    WarpDriverModelState,
    CustomModelState>;
static_assert(
    EachAlternativeIsModelAgentState<OperationalModelState>,
    "Every agent model state must provide a 'Point position' member");

inline Point& Pos(OperationalModelState& state)
{
    return std::visit([](auto& m) -> Point& { return m.position; }, state);
}

inline const Point& Pos(const OperationalModelState& state)
{
    return std::visit([](const auto& m) -> const Point& { return m.position; }, state);
}

/// Maps agent model state to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType ModelTypeOf(const OperationalModelState& state)
{
    return std::visit(
        overloaded{
            [](const GeneralizedCentrifugalForceModelState&) {
                return OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE;
            },
            [](const CollisionFreeSpeedModelState&) {
                return OperationalModelType::COLLISION_FREE_SPEED;
            },
            [](const CollisionFreeSpeedModelV2State&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V2;
            },
            [](const CollisionFreeSpeedModelV3State&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V3;
            },
            [](const AnticipationVelocityModelState&) {
                return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
            },
            [](const SocialForceModelState&) { return OperationalModelType::SOCIAL_FORCE; },
            [](const WarpDriverModelState&) { return OperationalModelType::WARP_DRIVER; },
            [](const CustomModelState&) { return OperationalModelType::CUSTOM_MODEL; }},
        state);
}
