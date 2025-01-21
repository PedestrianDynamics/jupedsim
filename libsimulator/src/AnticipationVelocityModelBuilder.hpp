// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AnticipationVelocityModel.hpp"

#include <stdint.h>

class AnticipationVelocityModelBuilder
{
public:
    AnticipationVelocityModelBuilder(uint64_t rng_seed);
    AnticipationVelocityModel Build();

private:
    uint64_t rng_seed{};
};
