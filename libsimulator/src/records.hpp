// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "geometry/location.hpp"
#include "stage.hpp"

#include <fmt/format.h>

#include <optional>
#include <variant>

/// An agent completed its target: a stage of its route, or the location it is steered to.
struct TargetCompletionRecord {
    GenericAgent::ID agent_id;
    std::variant<BaseStage::ID, Location> target;
    /// Simulation time in seconds.
    double time;
    /// Where the agent stood.
    Location location;
};

/// An agent left the simulation: through an exit, or by Simulation::mark_agent_for_removal.
struct RemovalRecord {
    GenericAgent::ID agent_id;
    /// std::nullopt after Simulation::mark_agent_for_removal.
    std::optional<BaseStage::ID> exit;
    /// Simulation time in seconds of the exit completion, or of the removal.
    double time;
};

template <>
struct fmt::formatter<TargetCompletionRecord> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const TargetCompletionRecord& r, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "TargetCompletionRecord(agent_id={}, target={}, time={}, location={})",
            r.agent_id,
            std::visit([](const auto& target) { return fmt::format("{}", target); }, r.target),
            r.time,
            r.location);
    }
};

template <>
struct fmt::formatter<RemovalRecord> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const RemovalRecord& r, FormatContext& ctx) const
    {
        if(r.exit) {
            return fmt::format_to(
                ctx.out(),
                "RemovalRecord(agent_id={}, exit={}, time={})",
                r.agent_id,
                *r.exit,
                r.time);
        }
        return fmt::format_to(ctx.out(), "RemovalRecord(agent_id={}, time={})", r.agent_id, r.time);
    }
};
