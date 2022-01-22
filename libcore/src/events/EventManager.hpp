#pragma once

#include "Event.hpp"
#include "IteratorPair.hpp"
#include "SimulationClock.hpp"

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
    bool HasEventsAfter(const SimulationClock & clock);

private:
    EventsContainerType _events;
};
