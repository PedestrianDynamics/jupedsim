#include "SubroomType.hpp"

template <>
SubroomType from_string(const std::string & str)
{
    if(str == "escalator_up") {
        return SubroomType::ESCALATOR_UP;
    }

    if(str == "escalator_down") {
        return SubroomType::ESCALATOR_DOWN;
    }

    if(str == "stair") {
        return SubroomType::STAIR;
    }

    if(str == "floor") {
        return SubroomType::FLOOR;
    }

    if(str == "corridor") {
        return SubroomType::CORRIDOR;
    }

    if(str == "entrance") {
        return SubroomType::ENTRANCE;
    }

    if(str == "lobby") {
        return SubroomType::LOBBY;
    }

    if(str == "dA") {
        return SubroomType::DA;
    }

    return SubroomType::UNKNOWN;
}
