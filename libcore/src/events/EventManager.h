#pragma once

#include "Event.h"
#include "IteratorPair.h"
#include "SimulationClock.h"

#include <chrono>
#include <map>
#include <set>
#include <utility>

class EventManager
{
public:
    using EventsContainerType    = std::multimap<std::chrono::nanoseconds, Event>;
    using ConstEventIteratorType = EventsContainerType::const_iterator;
    IteratorPair<ConstEventIteratorType> NextEvents(SimulationClock _clock);
    void add(Event event);

private:
    EventsContainerType _events;
};
