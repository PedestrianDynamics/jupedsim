// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModel.hpp"

#include <stdint.h>

class AnticipationVelocityModelBuilder
{
public:
    AnticipationVelocityModelBuilder(double pushoutStrength, uint64_t rng_seed);
    AnticipationVelocityModel Build();

private:
    double pushoutStrength{};
    uint64_t rng_seed{};
};
