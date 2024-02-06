// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include <memory>

struct Geometry {
    std::unique_ptr<CollisionGeometry> collisionGeometry;
    std::unique_ptr<RoutingEngine> routingEngine;
};
