// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Point.hpp"

#include <optional>

struct SocialForceModelUpdate {
    Point position{};
    Point velocity{};
};