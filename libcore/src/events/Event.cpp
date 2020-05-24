#include "Event.h"

Event::Event(double time, EventAction action) : _time(time), _action(action) {}

std::ostream & operator<<(std::ostream & out, const Event & event)
{
    return out << event.ToString();
}