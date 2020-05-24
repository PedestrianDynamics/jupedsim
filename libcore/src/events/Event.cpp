#include "Event.h"

Event::Event(double time) : _time(time) {}

std::ostream & operator<<(std::ostream & out, const Event & event)
{
    return out << event.ToString();
}