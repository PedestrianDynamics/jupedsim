// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "OperationalModels/OperationalModelState.hpp"
#include "OperationalModels/OperationalModelType.hpp"
#include "StrategicalModelState.hpp"
#include "TacticalModelState.hpp"
#include "UniqueID.hpp"
#include "Visitor.hpp"

#include <fmt/core.h>

#include <concepts>
#include <deque>
#include <utility>

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

struct GenericAgent;
const Point& Pos(const GenericAgent& agent);
Point& Pos(GenericAgent& agent);

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    StrategicalModelState strategical{};
    TacticalModelState tactical{};

    static_assert(
        EachAlternativeIsModelAgentState<OperationalModelState>,
        "Every agent model state must provide a 'Point position' member");
    OperationalModelState state{};

    Point& position() { return Pos(*this); }
    const Point& position() const { return Pos(*this); }

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        OperationalModelState state_)
        : strategical{.journeyId = journeyId_, .stageId = stageId_}, state(std::move(state_))
    {
        // The position is owned by the state; prime the strategical target with it.
        strategical.target = Pos(*this);
        // Passing Invalid requests a fresh unique id. Copies of the same state must not
        // share an id, so the state's own id is not reused.
        id = id_ != ID::Invalid ? id_ : ID{};
    }
};

inline const Point& Pos(const GenericAgent& agent)
{
    return std::visit([](const auto& m) -> const Point& { return m.position; }, agent.state);
}

inline Point& Pos(GenericAgent& agent)
{
    return std::visit([](auto& m) -> Point& { return m.position; }, agent.state);
}

inline const Point& Pos(const OperationalModelState& state)
{
    return std::visit([](const auto& s) -> const Point& { return s.position; }, state);
}

inline Point& Pos(OperationalModelState& state)
{
    return std::visit([](auto& s) -> Point& { return s.position; }, state);
}

/// Maps per-agent model state to the operational model type it belongs to.
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

template <class Agent>
using AgentContainer = std::deque<Agent>;

template <>
struct fmt::formatter<GenericAgent> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GenericAgent& agent, FormatContext& ctx) const
    {
        return std::visit(
            [&ctx, &agent](const auto& s) {
                return fmt::format_to(
                    ctx.out(),
                    "Agent[id={}, journey={}, stage={}, destination={}, target={}, pos={}, "
                    "state={}]",
                    agent.id,
                    agent.strategical.journeyId,
                    agent.strategical.stageId,
                    agent.tactical.destination,
                    agent.strategical.target,
                    Pos(agent),
                    s);
            },
            agent.state);
    }
};
