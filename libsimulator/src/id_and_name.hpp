// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/format.h>

#include <optional>
#include <string>

/// @p id, followed by @p name in parentheses if there is one, e.g. "3 (wp1)". Messages put the
/// kind in front: "Target 3 (wp1) is an exit".
template <typename ID>
std::string id_and_name(const ID& id, const std::optional<std::string>& name)
{
    if(name) {
        return fmt::format("{} ({})", id, *name);
    }
    return fmt::format("{}", id);
}

/// id_and_name() of @p named, e.g. a stage or a route.
template <typename Named>
std::string id_and_name(const Named& named)
{
    return id_and_name(named.id(), named.name());
}
