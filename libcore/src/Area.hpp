#pragma once

#include "ConvexPolygon.hpp"
#include "util/UniqueID.hpp"
#include <geometry/Point.hpp>

#include <set>
#include <string>

struct Area {
    using Id = uint32_t;
    std::set<std::string> lables;
    ConvexPolygon polygon;
};
