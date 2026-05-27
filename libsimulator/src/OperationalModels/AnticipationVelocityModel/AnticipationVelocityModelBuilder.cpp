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

AnticipationVelocityModel AnticipationVelocityModelBuilder::Build()
{
    return AnticipationVelocityModel(pushoutStrength, rng_seed);
}
