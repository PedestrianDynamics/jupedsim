// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "WarpDriverModel.hpp"

#include <cstdint>

class WarpDriverModelBuilder
{
    double _timeHorizon;
    double _stepSize;
    double _sigma;
    double _timeUncertainty;
    double _velocityUncertaintyX;
    double _velocityUncertaintyY;
    int _numSamples;
    uint64_t _rngSeed;

public:
    WarpDriverModelBuilder(
        double timeHorizon = 2.0,
        double stepSize = 0.5,
        double sigma = 0.3,
        double timeUncertainty = 0.5,
        double velocityUncertaintyX = 0.2,
        double velocityUncertaintyY = 0.2,
        int numSamples = 20,
        uint64_t rngSeed = 42);
    WarpDriverModel Build();
};
