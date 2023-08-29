// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"
#include "VelocityModelData.hpp"

#include <memory>
class Journey;
class BaseStage;

struct GenericAgent {
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    jps::UniqueID<Journey> journeyId{jps::UniqueID<Journey>::Invalid};
    size_t currentJourneyStageIdx{};
    jps::UniqueID<BaseStage> stageId{jps::UniqueID<BaseStage>::Invalid};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};
    OperationalModel::ParametersID parameterProfileId{OperationalModel::ParametersID::Invalid};

    // Agent fields common for all models
    Point pos{};
    Point orientation{};

    using Model = std::variant<GeneralizedCentrifugalForceModelData, VelocityModelData>;
    Model model{};

    GenericAgent(
        ID id_,
        jps::UniqueID<Journey> journeyId_,
        OperationalModel::ParametersID parameterProfileId_,
        Point pos_,
        Point orientation_,
        Model model_)
        : id(id_ != ID::Invalid ? id_ : ID{})
        , journeyId(journeyId_)
        , parameterProfileId(parameterProfileId_)
        , pos(pos_)
        , orientation(orientation_)
        , model(std::move(model_))
    {
    }
};
