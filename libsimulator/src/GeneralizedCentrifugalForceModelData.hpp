// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
struct GeneralizedCentrifugalForceModelData {
    double speed{};
    Point e0{};
    int orientationDelay{};
    double mass{};
    double tau{};
    double v0{};
    double Av{};
    double AMin{};
    double BMin{};
    double BMax;
};
