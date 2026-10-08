// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Destination.hpp"
#include "Geometry/Location.hpp"

#include <variant>

using RoutingTarget = std::variant<Destination, Location>;
