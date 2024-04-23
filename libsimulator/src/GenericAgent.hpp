// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "CollisionFreeSpeedModelData.hpp"
#include "CollisionFreeSpeedModelV2Data.hpp"
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "SocialForceModelData.hpp"
#include "UniqueID.hpp"
#include "Visitor.hpp"

#include <memory>
class Journey;
class BaseStage;

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};

    // This is evaluated by the "operational level"
    Point destination{};
    // TODO(kkratz): Type needs to be BaseStage::Location
    std::variant<Point, jps::UniqueID<BaseStage>> target{};

    // Agent fields common for all models
    Point pos{};
    Point orientation{1, 0};

    using Model = std::variant<
        GeneralizedCentrifugalForceModelData,
        CollisionFreeSpeedModelData,
        CollisionFreeSpeedModelV2Data,
        SocialForceModelData>;
    Model model{};

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        jps::UniqueID<BaseStage> stageId_,
        Point pos_,
        Point orientation_,
        Model model_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , stageId(stageId_)
        , target(pos_)
        , pos(pos_)
        , orientation(orientation_)
        , model(std::move(model_))
    {
    }
};
template <>
struct fmt::formatter<GenericAgent> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const GenericAgent& agent, FormatContext& ctx) const
    {
        return std::visit(
            overloaded{
                [&ctx, &agent](const GeneralizedCentrifugalForceModelData& m) {
                    return fmt::format_to(
                        ctx.out(),
                        "Agent[id={}, journey={}, stage={}, destination={}, waypoint={}, pos={}, "
                        "orientation={}, model={})",
                        agent.id,
                        agent.journeyId,
                        agent.stageId,
                        agent.destination,
                        agent.target,
                        agent.pos,
                        agent.orientation,
                        m);
                },
                [&ctx, &agent](const CollisionFreeSpeedModelData& m) {
                    return fmt::format_to(
                        ctx.out(),
                        "Agent[id={}, journey={}, stage={}, destination={}, waypoint={}, pos={}, "
                        "orientation={}, model={})",
                        agent.id,
                        agent.journeyId,
                        agent.stageId,
                        agent.destination,
                        agent.target,
                        agent.pos,
                        agent.orientation,
                        m);
                }},
            [&ctx, &agent](const SocialForceModelData& m) {
                return fmt::format_to(
                    ctx.out(),
                    "Agent[id={}, journey={}, stage={}, destination={}, waypoint={}, pos={}, "
                    "orientation={}, model={})",
                    agent.id,
                    agent.journeyId,
                    agent.stageId,
                    agent.destination,
                    agent.target,
                    agent.pos,
                    agent.orientation,
                    m);
            },
            agent.model);
    }
};
