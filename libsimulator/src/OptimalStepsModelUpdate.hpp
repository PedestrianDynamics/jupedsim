// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

struct OptimalStepsModelUpdate {
    Point position{};
    Point orientation{};

    double nextTimeToAct{};
};
