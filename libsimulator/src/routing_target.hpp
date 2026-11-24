// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "geometry/location.hpp"
#include "visitor.hpp"

#include <fmt/core.h>

#include <variant>

using RoutingTarget = std::variant<Destination, Location>;

/// How close an agent must come to a location it is steered to, to have reached it, in m.
/// Reaching it does not change how the agent moves: it keeps steering to the location.
constexpr double steering_arrival_tolerance = 0.05;

/// Whether an agent at @p where has reached @p target: same region, and within
/// steering_arrival_tolerance.
inline bool has_reached(const Location& where, const Location& target)
{
    return where.region() == target.region() &&
           (target.xy() - where.xy()).norm() <= steering_arrival_tolerance;
}

template <>
struct fmt::formatter<RoutingTarget> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const RoutingTarget& target, FormatContext& ctx) const
    {
        return std::visit(
            Overloaded{
                [&ctx](const Destination& d) { return fmt::format_to(ctx.out(), "{}", d); },
                [&ctx](const Location& l) { return fmt::format_to(ctx.out(), "{}", l); }},
            target);
    }
};
