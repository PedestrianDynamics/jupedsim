// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "geometry/location.hpp"
#include "visitor.hpp"

#include <fmt/core.h>

#include <variant>

using RoutingTarget = std::variant<Destination, Location>;

/// How close an agent must come to a point target (direct steering) to have reached it, in m.
/// Agents do not slow down before a point, so they overshoot it by up to one step
/// (desired speed * dt) and oscillate around it; the tolerance must be at least half a step for
/// them to reach it. 0.05 m covers steps up to 0.1 m, e.g. 1.34 m/s with dt up to 0.07 s.
/// Reaching the point does not change how the agent moves: it keeps steering to the point.
constexpr double point_arrival_tolerance = 0.05;

/// Whether @p where has reached the point target @p point: same region, horizontal distance at
/// most point_arrival_tolerance.
inline bool has_reached(const Location& where, const Location& point)
{
    return where.region() == point.region() &&
           (point.xy() - where.xy()).norm() <= point_arrival_tolerance;
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
