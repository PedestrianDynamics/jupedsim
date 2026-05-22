// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "RoutingEngine.hpp"

#include <functional>
#include <memory>

// Opaque wrapper around a RoutingEngine factory function so the factory can be
// constructed in C++ and passed through Python as a single object, without ever
// transferring ownership of a RoutingEngine across the Python/C++ boundary.
struct RoutingFactory {
    std::function<std::unique_ptr<RoutingEngine>(const PolyWithHoles&)> func;
};
