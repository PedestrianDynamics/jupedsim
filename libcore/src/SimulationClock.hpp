#pragma once

#include <chrono>

class SimulationClock
{
    uint64_t _iteration{0};
    double _dT;

public:
    explicit SimulationClock(double dT);

    void Advance();

    double ElapsedTime() const;

    uint64_t Iteration() const;

    double dT() const;
};
