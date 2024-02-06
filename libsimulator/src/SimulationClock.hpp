// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
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
