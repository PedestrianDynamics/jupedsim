// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Stage.hpp"
#include "UniqueID.hpp"

#include <fmt/core.h>

struct NotifyWaitingSet {
    Stage::ID stageId;
    NotifiableWaitingSet::WaitingState newState;
};

struct NotifyQueue {
    Stage::ID stageId;
    size_t count;
};

using Event = std::variant<NotifyWaitingSet, NotifyQueue>;

namespace fmt
{
template <>
struct formatter<NotifyWaitingSet> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& p_ctx)
    {
        return p_ctx.begin();
    }

    template <typename FormatContext>
    auto format(NotifyWaitingSet const& evt, FormatContext& p_ctx) const
    {
        return fmt::format_to(
            p_ctx.out(),
            "NotifyWaitingSet[stage id: {}, state: {}]",
            evt.stageId,
            evt.newState == NotifiableWaitingSet::WaitingState::Active ? "active" : "inactive");
    }
};

template <>
struct formatter<NotifyQueue> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& p_ctx)
    {
        return p_ctx.begin();
    }

    template <typename FormatContext>
    auto format(NotifyQueue const& evt, FormatContext& p_ctx) const
    {
        return fmt::format_to(
            p_ctx.out(), "NotifyQueue[stage id: {}, count: {}]", evt.stageId, evt.count);
    }
};
} // namespace fmt
