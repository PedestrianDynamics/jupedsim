#include "RoutingStrategy.hpp"

#include <stdexcept>


template <>
RoutingStrategy from_string<RoutingStrategy>(const std::string & string)
{
    if(string == "global_shortest") {
        return RoutingStrategy::ROUTING_GLOBAL_SHORTEST;
    }
    if(string == "ff_global_shortest") {
        return RoutingStrategy::ROUTING_FF_GLOBAL_SHORTEST;
    }
    return RoutingStrategy::UNKNOWN;
}
