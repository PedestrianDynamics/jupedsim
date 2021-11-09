#pragma once

#include "Simulation.h"
#include "events/Event.h"

#include <chrono>

std::chrono::nanoseconds EventMinTime(Event event);

void ProcessEvent(CreatePedestrianEvent event, Simulation & sim);
void ProcessEvent(DummyEvent event, Simulation & sim);
