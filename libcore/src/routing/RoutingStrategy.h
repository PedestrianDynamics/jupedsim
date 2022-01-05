#pragma once

#include "Enum.h"

#include <string>

enum class RoutingStrategy { ROUTING_GLOBAL_SHORTEST, ROUTING_FF_GLOBAL_SHORTEST, UNKNOWN };


template <>
RoutingStrategy from_string<RoutingStrategy>(const std::string & string);
