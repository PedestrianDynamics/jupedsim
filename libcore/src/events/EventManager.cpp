#include "EventManager.hpp"

#include "events/EventVisitors.hpp"

#include <chrono>


IteratorPair<EventManager::ConstEventIteratorType> EventManager::NextEvents(SimulationClock _clock)
{
    auto ub = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(_clock.ElapsedTime()));
    auto lb_exclusive = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(_clock.ElapsedTime() - _clock.dT()));

    return {_events.upper_bound(lb_exclusive), _events.upper_bound(ub)};
}

void EventManager::add(const Event & event)
{
    _events.insert({EventMinTime(event), event});
}

bool EventManager::HasEventsAfter(const SimulationClock & clock)
{
    return _events.lower_bound(std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::duration<double>(clock.ElapsedTime() - clock.dT()))) != _events.end();
}
