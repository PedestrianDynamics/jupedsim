#pragma once

#include "Event.h"
#include "SimulationClock.h"

#include <set>
#include <utility>

class EventManager
{
public:
    using EventsContainerType    = std::multiset<Event>;
    using ConstEventIteratorType = EventsContainerType::const_iterator;
    std::pair<ConstEventIteratorType, ConstEventIteratorType> NextEvents(SimulationClock);

private:
    EventsContainerType _events;
};
