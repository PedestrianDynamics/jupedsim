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

#include <deque>
#include <utility>
#include <variant>
class Journey;
class BaseStage;

using OperationalModelStateVariant = std::variant<
    GeneralizedCentrifugalForceModel::State,
    CollisionFreeSpeedModel::State,
    CollisionFreeSpeedModelV2::State,
    CollisionFreeSpeedModelV3::State,
    AnticipationVelocityModel::State,
    SocialForceModel::State,
    WarpDriverModel::State,
    CustomModel::State>;

/// We need this extra class to be able to forward-declare. Otherwise we run
/// into a circular dependency.
struct OperationalModelState : OperationalModelStateVariant {
    using OperationalModelStateVariant::variant;

    OperationalModelState(OperationalModelStateVariant state)
        : OperationalModelStateVariant(std::move(state))
    {
    }
};

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};

    // This is evaluated by the "operational level"
    Point nextTarget{};
    Point finalTarget{};

    OperationalModelState state{};

    Point Position() const { return _position; }
    void MoveAlongSurface(Point delta) { _position += delta; }

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        Point position_,
        OperationalModelState state_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , stageId(stageId_)
        , finalTarget(position_)
        , state(std::move(state_))
        , _position(position_)
    {
    }

private:
    Point _position{};
};

/// Maps agent model data to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType ModelTypeOf(const OperationalModelState& model)
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
                    "state={})",
                    agent.id,
                    agent.journeyId,
                    agent.stageId,
                    agent.nextTarget,
                    agent.finalTarget,
                    agent.Position(),
                    m);
            },
            agent.state);
    }
};
