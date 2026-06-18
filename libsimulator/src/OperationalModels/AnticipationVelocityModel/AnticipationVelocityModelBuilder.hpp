// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

#include <stdint.h>

#include <memory>

class AnticipationVelocityModelBuilder
{
public:
    AnticipationVelocityModelBuilder(double pushoutStrength, uint64_t rng_seed);
    std::unique_ptr<OperationalModel> Build();

private:
    double pushoutStrength{};
    uint64_t rng_seed{};
};
