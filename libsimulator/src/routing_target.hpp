// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "geometry/location.hpp"
#include "visitor.hpp"

#include <fmt/core.h>

#include <variant>

using RoutingTarget = std::variant<Destination, Location>;

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
