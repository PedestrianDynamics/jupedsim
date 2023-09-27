// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include <optional>

struct GeneralizedCentrifugalForceModelUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    Point e0{}; // desired direction
};
