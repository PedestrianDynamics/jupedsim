#include "OldEvent.hpp"

OldEvent::OldEvent(double time) : _time(time) {}

std::ostream & operator<<(std::ostream & out, const OldEvent & event)
{
    return out << event.ToString();
}
