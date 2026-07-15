// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "UniqueID.hpp"

// Forward declarations only — UniqueID uses these as phantom tags and Stage.hpp
// includes GenericAgent.hpp, so including the full headers here would be circular.
class Journey;
class BaseStage;

struct StrategicalModelState {
    Point target{};
    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};
};
