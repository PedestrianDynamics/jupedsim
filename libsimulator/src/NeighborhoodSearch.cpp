/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "NeighborhoodSearch.hpp"

#include "Agent.hpp"
#include "NeighborhoodIterator.hpp"
#include "Point.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <shared_mutex>

std::shared_mutex grid_mutex;

NeighborhoodSearch::NeighborhoodSearch(double cellSize) : _cellSize(cellSize)
{
}

NeighborhoodSearch::~NeighborhoodSearch()
{
}

void NeighborhoodSearch::Update(const std::vector<std::unique_ptr<Agent>>& peds)
{
    std::unique_lock exclusive_lock(grid_mutex);

    std::vector<Grid2D<Agent*>::IndexValuePair> values;
    values.reserve(peds.size());
    for(const auto& ped : peds) {
        // determine the cell coordinates of pedestrian i
        std::int32_t ix = static_cast<std::int32_t>(ped->GetPos().x / _cellSize);
        std::int32_t iy = static_cast<std::int32_t>(ped->GetPos().y / _cellSize);
        values.push_back({{ix, iy}, ped.get()});
    }
    _grid = Grid2D<Agent*>(values);
}

IteratorPair<NeighborhoodIterator, NeighborhoodEndIterator>
NeighborhoodSearch::GetNeighboringAgents(Point pos, double radius) const
{
    std::int32_t pos_idx = static_cast<std::int32_t>(pos.x / _cellSize);
    std::int32_t pos_idy = static_cast<std::int32_t>(pos.y / _cellSize);

    std::int32_t nh_level = static_cast<std::int32_t>(std::ceil(radius / _cellSize));

    auto filter = [pos, radius](Point other_pos) { return Distance(pos, other_pos) < radius; };

    return {
        {_grid,
         pos_idx - nh_level,
         pos_idy - nh_level,
         pos_idx + nh_level,
         pos_idy + nh_level,
         filter},
        {}};
}
