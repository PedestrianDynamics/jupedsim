// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"
#include "VelocityModelData.hpp"
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
    Point waypoint{};

    // Agent fields common for all models
    Point pos{};
    Point orientation{};

    using Model = std::variant<GeneralizedCentrifugalForceModelData, VelocityModelData>;
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
                        agent.waypoint,
                        agent.pos,
                        agent.orientation,
                        m);
                },
                [&ctx, &agent](const VelocityModelData& m) {
                    return fmt::format_to(
                        ctx.out(),
                        "Agent[id={}, journey={}, stage={}, destination={}, waypoint={}, pos={}, "
                        "orientation={}, model={})",
                        agent.id,
                        agent.journeyId,
                        agent.stageId,
                        agent.destination,
                        agent.waypoint,
                        agent.pos,
                        agent.orientation,
                        m);
                }},
            agent.model);
    }
};
