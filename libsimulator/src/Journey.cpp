// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include "GenericAgent.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "TemplateHelper.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "fmt/ranges.h"
