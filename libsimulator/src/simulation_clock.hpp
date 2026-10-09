// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstdint>

class SimulationClock
{
    uint64_t _iteration{0};
    double _dt;

public:
    explicit SimulationClock(double dt);

    void advance();

    double elapsed_time() const;

    uint64_t iteration() const;

    double dt() const;
};
