#pragma once

#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include <memory>

struct Geometry {
    std::unique_ptr<CollisionGeometry> collisionGeometry;
    std::unique_ptr<RoutingEngine> routingEngine;
};
