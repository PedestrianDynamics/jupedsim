// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include "GenericAgent.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "TemplateHelper.hpp"
#include "fmt/ranges.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>
