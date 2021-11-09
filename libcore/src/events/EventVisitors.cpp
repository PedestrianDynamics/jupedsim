#include "EventVisitors.h"

#include "events/Event.h"

#include <variant>

std::chrono::nanoseconds EventMinTime(Event event)
{
    auto call = [](auto const & obj) { return obj.MinTime(); };
    return std::visit(call, event);
}


void ProcessEvent(CreatePedestrianEvent event, Simulation & sim) {}
void ProcessEvent(DummyEvent event, Simulation & sim) {}
