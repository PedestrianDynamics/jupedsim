// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
struct GeneralizedCentrifugalForceModelData {
    double speed{};
    Point e0{};
    int orientationDelay{};
};
