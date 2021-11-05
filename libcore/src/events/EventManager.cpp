#include "EventManager.h"


std::pair<EventManager::ConstEventIteratorType, EventManager::ConstEventIteratorType>
    EventManager::NextEvents(SimulationClock)
{
    return {_events.cbegin(), _events.cend()};
}
