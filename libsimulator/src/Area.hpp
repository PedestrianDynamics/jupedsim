#pragma once

#include "ConvexPolygon.hpp"
#include "UniqueID.hpp"

#include <set>
#include <string>

struct Area {
    using Id = uint32_t;
    std::set<std::string> lables;
    ConvexPolygon polygon;
};
