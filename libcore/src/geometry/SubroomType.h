#pragma once

#include "Enum.h"

#include <string>

enum class SubroomType {
    ESCALATOR_UP,
    ESCALATOR_DOWN,
    STAIR,
    FLOOR,
    CORRIDOR,
    ENTRANCE,
    LOBBY,
    DA, // used in ff router. meaning unknown
    UNKNOWN
};

template <>
SubroomType from_string(const std::string & str);
