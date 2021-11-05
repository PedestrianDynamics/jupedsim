#pragma once

#include <chrono>

class SimulationTimer
{
    uint64_t _iteration{0};
    double _dT;

public:
    explicit SimulationTimer(double dT);

    void Advance();

    double ElapsedTime() const;

    uint64_t Iteration() const;

    double dT() const;
};
