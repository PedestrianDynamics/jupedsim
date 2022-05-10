#pragma once

#include "RoutingEngine.hpp"
#include "geometry/Building.hpp"

NavMeshRoutingEngine MakeFromBuilding(const Building& building);
