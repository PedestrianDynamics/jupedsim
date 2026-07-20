// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "AnticipationVelocityModel.hpp"
#include "CollisionFreeSpeedModel.hpp"
#include "CollisionFreeSpeedModelV2.hpp"
#include "CollisionFreeSpeedModelV3.hpp"
#include "GeneralizedCentrifugalForceModel.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "OperationalModels/OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModel.hpp"
#include "UniqueID.hpp"
#include "Visitor.hpp"
#include "WarpDriver/WarpDriverModel.hpp"

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
    Point endTarget{};

    using ModelState = std::variant<
        GeneralizedCentrifugalForceModel::State,
        CollisionFreeSpeedModel::State,
        CollisionFreeSpeedModelV2::State,
        CollisionFreeSpeedModelV3::State,
        AnticipationVelocityModel::State,
        SocialForceModel::State,
        WarpDriverModel::State,
        CustomModel::State>;
    static_assert(
        EachAlternativeIsModelAgentState<ModelState>,
        "Every agent model state must provide a 'Point position' member");
    ModelState model{};

    Point& position()
    {
        return std::visit([](auto& m) -> Point& { return m.position; }, model);
    }
    const Point& position() const
    {
        return std::visit([](const auto& m) -> const Point& { return m.position; }, model);
    }

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        ModelState model_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , stageId(stageId_)
        , model(std::move(model_))
    {
        // Position is owned by the model state; seed the initial waypoint from it.
        endTarget = position();
    }
};

/// Maps agent model data to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType ModelTypeOf(const GenericAgent::ModelState& model)
{
    return std::visit(
        overloaded{
            [](const GeneralizedCentrifugalForceModel::State&) {
                return OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE;
            },
            [](const CollisionFreeSpeedModel::State&) {
                return OperationalModelType::COLLISION_FREE_SPEED;
            },
            [](const CollisionFreeSpeedModelV2::State&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V2;
            },
            [](const CollisionFreeSpeedModelV3::State&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V3;
            },
            [](const AnticipationVelocityModel::State&) {
                return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
            },
            [](const SocialForceModel::State&) { return OperationalModelType::SOCIAL_FORCE; },
            [](const WarpDriverModel::State&) { return OperationalModelType::WARP_DRIVER; },
            [](const CustomModel::State&) { return OperationalModelType::CUSTOM_MODEL; }},
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
                    agent.endTarget,
                    agent.position(),
                    m);
            },
            agent.model);
    }
};
