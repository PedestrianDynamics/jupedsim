// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation_clock.hpp"

#include <cstdint>

SimulationClock::SimulationClock(double dt) : _dt(dt)
{
}

void SimulationClock::advance()
{
    ++_iteration;
}

double SimulationClock::elapsed_time() const
{
    return _dt * _iteration;
}

uint64_t SimulationClock::iteration() const
{
    return _iteration;
}

double SimulationClock::dt() const
{
    return _dt;
}
