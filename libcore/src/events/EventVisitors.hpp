#pragma once

#include "Simulation.hpp"
#include "events/Event.hpp"

#include <chrono>

std::chrono::nanoseconds EventMinTime(Event event);

void ProcessEvent(CreatePedestrianEvent event, Simulation & sim);
void ProcessEvent(DummyEvent event, Simulation & sim);
