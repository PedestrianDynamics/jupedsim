// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "WarpDriverModel.hpp"

class WarpDriverModelBuilder
{
    double _timeHorizon;
    double _stepSize;
    double _sigma;
    double _timeUncertainty;
    double _velocityUncertaintyX;
    double _velocityUncertaintyY;

public:
    WarpDriverModelBuilder(
        double timeHorizon = 2.0,
        double stepSize = 0.5,
        double sigma = 0.3,
        double timeUncertainty = 0.5,
        double velocityUncertaintyX = 0.2,
        double velocityUncertaintyY = 0.2);
    WarpDriverModel Build();
};
