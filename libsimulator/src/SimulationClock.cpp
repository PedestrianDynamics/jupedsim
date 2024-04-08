// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SimulationClock.hpp"

#include "SimulationError.hpp"

SimulationClock::SimulationClock(double dT) : _dT(dT)
{
}

void SimulationClock::Advance()
{
    ++_iteration;
}

double SimulationClock::ElapsedTime() const
{
    return _dT * _iteration;
}

uint64_t SimulationClock::Iteration() const
{
    return _iteration;
}

double SimulationClock::dT() const
{
    return _dT;
}

void SimulationClock::UpdateDT(double newDT)
{
    if(newDT < 0.0) {
        throw SimulationError("Simulation dt may only be positive");
    }
    _dT = newDT;
}
