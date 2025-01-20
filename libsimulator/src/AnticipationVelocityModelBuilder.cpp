// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AnticipationVelocityModelBuilder.hpp"

AnticipationVelocityModelBuilder::AnticipationVelocityModelBuilder(uint64_t rng_seed)
    : rng_seed(rng_seed)
{
}

AnticipationVelocityModel AnticipationVelocityModelBuilder::Build()
{
    return AnticipationVelocityModel(rng_seed);
}
