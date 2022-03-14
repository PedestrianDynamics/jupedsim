#pragma once

#include "DoorState.hpp"
#include "geometry/Line.hpp"

struct Door {
    Line linesegment;
    int id;
    DoorState state;
};
