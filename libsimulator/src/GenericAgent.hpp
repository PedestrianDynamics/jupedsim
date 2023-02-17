/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <memory>

class GenericAgent
{
public:
    using ID = jps::UniqueID<GenericAgent>;
    ID id{};

    // This is evaluated by the "strategic level"
    std::unique_ptr<Behaviour> behaviour{};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};
    OperationalModel::ParametersID parameterProfileId{OperationalModel::ParametersID::Invalid};

    // Agent fields common for all models
    Point pos{};
    Point orientation{};
};

// TODO(kkratz): This does neither belong here nor is this a good name.
Point MollifyE0(
    const Point& target,
    const Point& pos,
    double deltaT,
    int orientationDelay,
    const Point& e0);
