#include "EventManager.h"

#include "events/EventVisitors.h"

#include <chrono>


IteratorPair<EventManager::ConstEventIteratorType> EventManager::NextEvents(SimulationClock _clock)
{
    auto ub = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(_clock.ElapsedTime()));
    auto lb = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(_clock.ElapsedTime() - _clock.dT()));

    return {_events.lower_bound(lb), _events.upper_bound(ub)};
}

void EventManager::add(Event event)
{
    _events.insert({EventMinTime(event), event});
}
