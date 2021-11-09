#pragma once

#include "general/Configuration.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <memory>
#include <vector>

/// TODO(kkratz) doc
std::vector<std::unique_ptr<Pedestrian>> CreateAllPedestrians(Building * building);

std::vector<std::unique_ptr<Pedestrian>>
CreateInitialPedestrians(Configuration & configuration, Building * building);
