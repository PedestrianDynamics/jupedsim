// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "AnticipationVelocityModelData.hpp"
#include "CollisionFreeSpeedModelData.hpp"
#include "CollisionFreeSpeedModelV2Data.hpp"
#include "CollisionFreeSpeedModelV3Data.hpp"
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModelData.hpp"
#include "OperationalModels/OperationalModelType.hpp"
#include "Point.hpp"
#include "SocialForceModelData.hpp"
#include "UniqueID.hpp"
#include "Visitor.hpp"
#include "WarpDriverModelData.hpp"

#include <fmt/core.h>

#include <deque>
#include <utility>
#include <variant>
class Journey;
class BaseStage;

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};

    // This is evaluated by the "operational level"
    Point destination{};
    Point target{};

    // Agent fields common for all models
    Point pos{};

    using Model = std::variant<
        GeneralizedCentrifugalForceModelData,
        CollisionFreeSpeedModelData,
        CollisionFreeSpeedModelV2Data,
        CollisionFreeSpeedModelV3Data,
        AnticipationVelocityModelData,
        SocialForceModelData,
        WarpDriverModelData,
        CustomModelData>;
    Model model{};

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        Point pos_,
        Model model_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , stageId(stageId_)
        , target(pos_)
        , pos(pos_)
        , model(std::move(model_))
    {
    }
};

/// Maps agent model data to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType ModelTypeOf(const GenericAgent::Model& model)
{
    return std::visit(
        overloaded{
            [](const GeneralizedCentrifugalForceModelData&) {
                return OperationalModelType::GENERALIZED_CENTRIFUGAL_FORCE;
            },
            [](const CollisionFreeSpeedModelData&) {
                return OperationalModelType::COLLISION_FREE_SPEED;
            },
            [](const CollisionFreeSpeedModelV2Data&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V2;
            },
            [](const CollisionFreeSpeedModelV3Data&) {
                return OperationalModelType::COLLISION_FREE_SPEED_V3;
            },
            [](const AnticipationVelocityModelData&) {
                return OperationalModelType::ANTICIPATION_VELOCITY_MODEL;
            },
            [](const SocialForceModelData&) { return OperationalModelType::SOCIAL_FORCE; },
            [](const WarpDriverModelData&) { return OperationalModelType::WARP_DRIVER; },
            [](const CustomModelData&) { return OperationalModelType::CUSTOM_MODEL; }},
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
                    agent.destination,
                    agent.target,
                    agent.pos,
                    m);
            },
            agent.model);
    }
};
