/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Grid2D.hpp"
#include "IteratorPair.hpp"
#include "NeighborhoodIterator.hpp"
#include "Point.hpp"

class NeighborhoodSearch
{
    double _cellSize;
    Grid2D<Agent*> _grid{};

public:
    explicit NeighborhoodSearch(double cellSize);
    ~NeighborhoodSearch();
    NeighborhoodSearch(const NeighborhoodSearch&) = default;
    NeighborhoodSearch(NeighborhoodSearch&&) = default;
    NeighborhoodSearch& operator=(const NeighborhoodSearch&) = default;
    NeighborhoodSearch& operator=(NeighborhoodSearch&&) = default;

    /**
     *Update the cells occupation
     */
    void Update(const std::vector<std::unique_ptr<Agent>>& peds);

    IteratorPair<NeighborhoodIterator, NeighborhoodEndIterator>
    GetNeighboringAgents(Point pos, double radius) const;
};
