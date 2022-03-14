#include "DoorState.hpp"

#include <exception>

template <>
DoorState from_string(const std::string & name)
{
    if(name == "open") {
        return DoorState::OPEN;
    }

    if(name == "temp_close") {
        return DoorState::TEMP_CLOSE;
    }

    if(name == "close") {
        return DoorState::CLOSE;
    }

    throw std::runtime_error(fmt::format("Cannot create DoorState from string \"{name}\"", name));
};
