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

#include "pedestrian/Pedestrian.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <shared_mutex>


std::shared_mutex grid_mutex;

NeighborhoodSearch::NeighborhoodSearch(double cellSize) : _cellSize(cellSize) {}

NeighborhoodSearch::~NeighborhoodSearch() {}

void NeighborhoodSearch::Update(const std::vector<std::unique_ptr<Pedestrian>> & peds)
{
    std::unique_lock exclusive_lock(grid_mutex);

    std::vector<Grid2D<Pedestrian *>::IndexValuePair> values;
    values.reserve(peds.size());
    for(const auto & ped : peds) {
        // determine the cell coordinates of pedestrian i
        std::int32_t ix = static_cast<std::int32_t>(ped->GetPos().x / _cellSize);
        std::int32_t iy = static_cast<std::int32_t>(ped->GetPos().y / _cellSize);
        values.push_back({{ix, iy}, ped.get()});
    }
    _grid = Grid2D<Pedestrian *>(values);
}


std::vector<Pedestrian *> NeighborhoodSearch::GetNeighbourhood(const Pedestrian * ped) const
{
    std::vector<Pedestrian *> neighbourhood;

    double xPed = ped->GetPos().x;
    double yPed = ped->GetPos().y;

    std::int32_t l = static_cast<std::int32_t>(xPed / _cellSize);
    std::int32_t k = static_cast<std::int32_t>(yPed / _cellSize);

    /**
     * We only aquire a shared lock here for reading data in the grid.
     * It is not allowed to write data using a shared lock.
     * Otherwise thread safety is lost.
     */
    std::shared_lock shared_lock(grid_mutex);

    // all neighbor cells
    for(std::int32_t i = l - 1; i <= l + 1; ++i) {
        for(std::int32_t j = k - 1; j <= k + 1; ++j) {
            for(auto & other_ped : _grid.get({i, j})) {
                if(other_ped.value->GetUID() != ped->GetUID()) {
                    neighbourhood.push_back(other_ped.value);
                }
            }
        }
    }

    return neighbourhood;
}
