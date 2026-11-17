// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/geometry.hpp"

#include <fmt/format.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

/// The geometry the regions of one GeometryBuilder belong to, once it is built.
struct RegionScope {
    std::shared_ptr<const Geometry> geometry;
};

/// Handle to a region for Python. It exists before its geometry is built, so it refers to the
/// scope its builder shares with all its regions.
struct RegionHandle {
    std::shared_ptr<RegionScope> scope;
    std::size_t id;
    std::optional<std::string> name;
};

/// Handle to region @p id of @p geometry.
inline RegionHandle region_of(std::shared_ptr<const Geometry> geometry, std::size_t id)
{
    std::optional<std::string> name = geometry->region_name(id);
    return {std::make_shared<RegionScope>(std::move(geometry)), id, std::move(name)};
}

inline bool operator==(const RegionHandle& a, const RegionHandle& b)
{
    if(a.id != b.id) {
        return false;
    }
    if(a.scope->geometry && b.scope->geometry) {
        return a.scope->geometry == b.scope->geometry;
    }
    return a.scope == b.scope;
}

template <>
struct fmt::formatter<RegionHandle> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const RegionHandle& region, FormatContext& ctx) const
    {
        if(region.name) {
            return fmt::format_to(ctx.out(), "Region(id={}, name={})", region.id, *region.name);
        }
        return fmt::format_to(ctx.out(), "Region(id={})", region.id);
    }
};
