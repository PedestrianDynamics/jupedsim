// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "OperationalModels/OperationalModelState.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

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

    using ModelState = OperationalModelState;
    ModelState model{};

    Point& position() { return Pos(model); }
    const Point& position() const { return Pos(model); }

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
        target = position();
    }
};

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
                    agent.position(),
                    m);
            },
            agent.model);
    }
};
