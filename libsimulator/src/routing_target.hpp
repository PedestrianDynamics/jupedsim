// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "geometry/location.hpp"
#include "kernel.hpp"
#include "line_segment.hpp"
#include "visitor.hpp"

#include <fmt/core.h>

#include <variant>

using RoutingTarget = std::variant<Destination, Location>;

/// How close an agent must come to a point target (direct steering) to have reached it, in m.
/// Reaching the point does not change how the agent moves: it keeps steering to the point.
constexpr double point_arrival_tolerance = 0.05;

/// Whether an agent at @p where, whose last step was @p last_step, has reached the point target
/// @p point: same region, and the point lies within point_arrival_tolerance of that step (the
/// segment from where it was to @p where). Agents do not slow down before a point, so a step can
/// jump past it; counting the whole step makes arrival independent of the step length (speed
/// times dt).
inline bool has_reached(const Location& where, const Vector& last_step, const Location& point)
{
    if(where.region() != point.region()) {
        return false;
    }
    const Point to = where.xy();
    const Point from = to - last_step;
    const double distance = last_step.is_zero_length() ? (point.xy() - to).norm() :
                                                         LineSegment(from, to).dist_to(point.xy());
    return distance <= point_arrival_tolerance;
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
