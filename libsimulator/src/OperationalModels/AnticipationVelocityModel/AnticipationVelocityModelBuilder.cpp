// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModelBuilder.hpp"

#include "AnticipationVelocityModel.hpp"

#include <cstdint>

AnticipationVelocityModelBuilder::AnticipationVelocityModelBuilder(
    double pushoutStrength,
    uint64_t rng_seed)
    : pushoutStrength(pushoutStrength), rng_seed(rng_seed)
{
}

std::unique_ptr<OperationalModel> AnticipationVelocityModelBuilder::Build()
{
    return std::make_unique<AnticipationVelocityModel>(pushoutStrength, rng_seed);
}
