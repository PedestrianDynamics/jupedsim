/**
 * \file        NeighborhoodSearch.cpp
 * \date        Nov 16, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J?lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 **/
#include "NeighborhoodSearch.hpp"

#include "geometry/Point.hpp"
#include "neighborhood/NeighborhoodIterator.hpp"
#include "pedestrian/Pedestrian.hpp"

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

void NeighborhoodSearch::Update(const std::vector<std::unique_ptr<Pedestrian>>& peds)
{
    std::unique_lock exclusive_lock(grid_mutex);

    std::vector<Grid2D<Pedestrian*>::IndexValuePair> values;
    values.reserve(peds.size());
    for(const auto& ped : peds) {
        // determine the cell coordinates of pedestrian i
        std::int32_t ix = static_cast<std::int32_t>(ped->GetPos().x / _cellSize);
        std::int32_t iy = static_cast<std::int32_t>(ped->GetPos().y / _cellSize);
        values.push_back({{ix, iy}, ped.get()});
    }
    _grid = Grid2D<Pedestrian*>(values);
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
