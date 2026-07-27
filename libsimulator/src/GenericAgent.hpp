// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "OperationalModels/OperationalModelState.hpp"
#include "OperationalModels/OperationalModelType.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"
#include "Visitor.hpp"

#include <fmt/core.h>

#include <concepts>
#include <deque>
#include <utility>
#include <variant>
class Journey;
class BaseStage;

/// Agent position is owned by the per-model agent state. Every alternative of
/// GenericAgent::ModelState must satisfy this concept; the framework accesses the
/// position type-erased through GenericAgent::position().
template <typename T>
concept ModelAgentState = requires(T t) {
    // position() hands out mutable Point& into the state, so a convertible or const member
    // is not enough.
    { t.position } -> std::same_as<Point&>;
};

template <typename Variant>
inline constexpr bool EachAlternativeIsModelAgentState = false;
template <typename... Ts>
inline constexpr bool EachAlternativeIsModelAgentState<std::variant<Ts...>> =
    (ModelAgentState<Ts> && ...);

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};

    // This is evaluated by the "operational level"
    Point nextTarget{};
    Point finalTarget{};

    using ModelState = OperationalModelState;
    static_assert(
        EachAlternativeIsModelAgentState<ModelState>,
        "Every agent model state must provide a 'Point position' member");
    ModelState state{};

    Point& position()
    {
        return std::visit([](auto& m) -> Point& { return m.position; }, state);
    }
    const Point& position() const
    {
        return std::visit([](const auto& m) -> const Point& { return m.position; }, state);
    }

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        ModelState state_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , stageId(stageId_)
        , state(std::move(state_))
    {
        // Position is owned by the model state; seed the initial waypoint from it.
        finalTarget = position();
    }
};

/// Maps agent model data to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType ModelTypeOf(const GenericAgent::ModelState& model)
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
        model);
}

template <class Agent>
using AgentContainer = std::deque<Agent>;

template <>
struct fmt::formatter<GenericAgent> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GenericAgent& agent, FormatContext& ctx) const
    {
        return std::visit(
            [&ctx, &agent](const auto& m) {
                return fmt::format_to(
                    ctx.out(),
                    "Agent[id={}, journey={}, stage={}, destination={}, waypoint={}, pos={}, "
                    "model={})",
                    agent.id,
                    agent.journeyId,
                    agent.stageId,
                    agent.nextTarget,
                    agent.finalTarget,
                    agent.position(),
                    m);
            },
            agent.state);
    }
};
