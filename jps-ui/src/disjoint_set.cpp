// Copyright © 2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "disjoint_set.hpp"

#include <cassert>
#include <numeric>
#include <stdexcept>

DisjointSet::DisjointSet(size_t count_elements) : parent(count_elements)
{
    std::iota(std::begin(parent), std::end(parent), 0);
}

size_t DisjointSet::Find(size_t id)
{
    if(id >= parent.size()) {
        throw std::runtime_error("Unknown id");
    };
    if(parent[id] != id) {
        parent[id] = Find(id);
    }
    return parent[id];
}

void DisjointSet::Union(size_t id_a, size_t id_b)
{
    const auto a = Find(id_a);
    const auto b = Find(id_b);
    parent[b] = a;
}
