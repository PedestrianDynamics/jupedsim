// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModels/OperationalModelState.hpp"
#include "Point.hpp"

#include <vector>

class NeighborQuery
{
public:
    NeighborQuery() = default;
    NeighborQuery(const NeighborQuery&) = delete;
    NeighborQuery& operator=(const NeighborQuery&) = delete;
    NeighborQuery(NeighborQuery&&) = delete;
    NeighborQuery& operator=(NeighborQuery&&) = delete;
    virtual ~NeighborQuery() = default;

    /// Returns copies of the model states of the frozen current generation's agents within
    /// `radius` [m] of `position`, excluding the agent bound at construction. Any additional
    /// filtering (e.g. line-of-sight visibility) is defined by the implementation the
    /// simulation supplies.
    virtual std::vector<OperationalModelState> operator()(Point position, double radius) const = 0;
};
