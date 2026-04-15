// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct WarpDriverModelUpdate {
    Point position{};
    Point orientation{};
    double stuckTime{0.0};
    double anchorX{0.0};
    double anchorY{0.0};
    double detourTime{0.0};
    int detourSide{1};
};
