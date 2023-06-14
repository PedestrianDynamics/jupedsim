/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Stage.hpp"
#include "UniqueID.hpp"

#include <fmt/core.h>

struct NotifyWaitingSet {
    jps::UniqueID<Journey> journeyId;
    size_t stageIdx;
    NotifiableWaitingSet::WaitingState newState;
};

struct NotifyQueue {
    jps::UniqueID<Journey> journeyId;
    size_t stageIdx;
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
            "NotifyWaitingSet[id: {}, idx: {}, state: {}]",
            evt.journeyId,
            evt.stageIdx,
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
            p_ctx.out(),
            "NotifyQueue[id: {}, idx: {}, count: {}]",
            evt.journeyId,
            evt.stageIdx,
            evt.count);
    }
};
} // namespace fmt
