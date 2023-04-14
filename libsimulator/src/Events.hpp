/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Stage.hpp"
#include "UniqueID.hpp"

struct NotifyWaitingSet {
    jps::UniqueID<Journey> journeyId;
    size_t stageIdx;
    NotifiableWaitingSet::WaitingState newState;
};

using Event = std::variant<NotifyWaitingSet>;
