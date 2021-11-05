#include "SimulationTimer.h"

#include <chrono>

SimulationTimer::SimulationTimer(double dT) : _dT(dT) {}

void SimulationTimer::Advance()
{
    ++_iteration;
}

double SimulationTimer::ElapsedTime() const
{
    return _dT * _iteration;
}

uint64_t SimulationTimer::Iteration() const
{
    return _iteration;
}

double SimulationTimer::dT() const
{
    return _dT;
}
