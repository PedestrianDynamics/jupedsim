// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <random>

class Random
{
private:
    std::mt19937 mt;

public:
    explicit Random(uint seed) : mt(seed){};

    template <class RealType>
    RealType normalDistributen(RealType mean, RealType sd)
    {
        std::normal_distribution d{mean, sd};
        return d(mt);
    };
};
