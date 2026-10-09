// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "geometry/location.hpp"
#include "operational_models/operational_model_state.hpp"
#include "operational_models/operational_model_type.hpp"
#include "point.hpp"
#include "routing_target.hpp"
#include "unique_id.hpp"
#include "visitor.hpp"

#include <fmt/core.h>

#include <deque>
#include <optional>
#include <utility>
class Journey;
class BaseStage;

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journey_id{jps::UniqueID<Journey>::invalid};
    jps::UniqueID<BaseStage> stage_id{jps::UniqueID<BaseStage>::invalid};

    /// Where the agent stands. Only the geometry can say that, so only it can build one.
    Location location;

    // This is evaluated by the "operational level"
    /// Unit vector along the route to the final target. Zero if the agent has reached it.
    Point route_orientation{};
    RoutingTarget final_target;

    OperationalModelState state{};

    GenericAgent(
        ID id,
        jps::UniqueID<Journey> journey_id,
        jps::UniqueID<BaseStage> stage_id,
        Location location,
        OperationalModelState state)
        : id(id != ID::invalid ? id : ID{})
        , journey_id(journey_id)
        , stage_id(stage_id)
        , location(location)
        , final_target(location)
        , state(std::move(state))
    {
    }
};

/// Maps agent model data to the operational model type it belongs to. Kept
/// exhaustive on purpose: adding a model type will not compile until the
/// mapping is extended.
inline OperationalModelType model_type_of(const OperationalModelState& model)
{
    return std::visit(
        Overloaded{
            [](const GeneralizedCentrifugalForceModelState&) {
                return OperationalModelType::GeneralizedCentrifugalForce;
            },
            [](const CollisionFreeSpeedModelState&) {
                return OperationalModelType::CollisionFreeSpeed;
            },
            [](const CollisionFreeSpeedModelV2State&) {
                return OperationalModelType::CollisionFreeSpeedV2;
            },
            [](const CollisionFreeSpeedModelV3State&) {
                return OperationalModelType::CollisionFreeSpeedV3;
            },
            [](const AnticipationVelocityModelState&) {
                return OperationalModelType::AnticipationVelocityModel;
            },
            [](const SocialForceModelState&) { return OperationalModelType::SocialForce; },
            [](const WarpDriverModelState&) { return OperationalModelType::WarpDriver; },
            [](const CustomModelState&) { return OperationalModelType::CustomModel; }},
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
                    "Agent[id={}, journey={}, stage={}, route_orientation={}, target={}, pos={}, "
                    "state={})",
                    agent.id,
                    agent.journey_id,
                    agent.stage_id,
                    agent.route_orientation,
                    agent.final_target,
                    agent.location,
                    m);
            },
            agent.state);
    }
};
