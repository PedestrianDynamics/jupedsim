#include "EventHelper.h"


std::optional<EventAction> EventHelper::StringToEventAction(const std::string & input)
{
    std::string name(input);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(name == "open") {
        return EventAction::DOOR_OPEN;
    }

    if(name == "temp_close") {
        return EventAction::DOOR_TEMP_CLOSE;
    }

    if(name == "close") {
        return EventAction::DOOR_CLOSE;
    }

    if(name == "close") {
        return EventAction::DOOR_CLOSE;
    }

    if(name == "reset") {
        return EventAction::DOOR_RESET_USAGE;
    }

    return std::nullopt;
}
