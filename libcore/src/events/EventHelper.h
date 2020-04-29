#pragma once

#include "DoorEvent.h"
#include "Event.h"

#include <optional>
#include <string>

namespace EventHelper
{
/**
* Helper function to get the Action from the input.
* @param input Input string
* @return Action which is identified by \p input
 */
std::optional<EventAction> StringToEventAction(const std::string & input);

} // namespace EventHelper
