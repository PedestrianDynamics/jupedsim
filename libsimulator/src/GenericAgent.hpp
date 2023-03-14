/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <memory>

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    Journey::ID journeyId{Journey::ID::Invalid};
    size_t currentJourneyStage{};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};
    OperationalModel::ParametersID parameterProfileId{OperationalModel::ParametersID::Invalid};

    // Agent fields common for all models
    Point pos{};
    Point orientation{};

    GenericAgent(
        ID id_,
        Journey::ID journeyId_,
        OperationalModel::ParametersID parameterProfileId_,
        Point pos_,
        Point orientation_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , parameterProfileId(parameterProfileId_)
        , pos(pos_)
        , orientation(orientation_)
    {
    }
};
