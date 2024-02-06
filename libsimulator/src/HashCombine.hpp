// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstddef>

namespace jps
{
/// Combining two Hash values as implemented in Boost
inline std::size_t hash_combine(std::size_t seed1, std::size_t seed2)
{
    std::size_t seed = seed1;
    seed ^= seed2 + 0x9e3779b9 + (seed1 << 6) + (seed1 >> 2);

    return seed;
}
} // namespace jps
