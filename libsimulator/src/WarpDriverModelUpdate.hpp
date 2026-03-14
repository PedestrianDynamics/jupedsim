// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

struct WarpDriverModelUpdate {
    Point position{};
    Point orientation{};
    int jamCounter{0};
};
