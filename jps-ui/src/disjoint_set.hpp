// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <cstddef>
#include <vector>

class DisjointSet
{
    std::vector<size_t> parent{};

public:
    DisjointSet(size_t count_elements);
    ~DisjointSet() = default;
    size_t Find(size_t id);
    void Union(size_t id_a, size_t id_b);
};
