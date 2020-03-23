/**
 * \file        LCGrid.cpp
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
#include "LCGrid.h"

#include "pedestrian/Pedestrian.h"

#include <algorithm>
#include <iterator>
#include <shared_mutex>


std::shared_mutex grid_mutex;

LCGrid::LCGrid(
    double gridXmin,
    double gridXmax,
    double gridYmin,
    double gridYmax,
    double cellSize) :
    _gridXmin(gridXmin),
    _gridYmin(gridYmin),
    _cellSize(cellSize),
    _gridSizeX((int) ((gridXmax - _gridXmin) / _cellSize) + 1 + 2), // 1 dummy cell on each side
    _gridSizeY((int) ((gridYmax - _gridYmin) / _cellSize) + 1 + 2), // 1 dummy cell on each side
    grid(_gridSizeY, _gridSizeX)
{
}

LCGrid::~LCGrid() {}

void LCGrid::Update(const std::vector<Pedestrian *> & peds)
{
    std::unique_lock exclusive_lock(grid_mutex);
    ClearGrid();

    for(auto & ped : peds) {
        // determine the cell coordinates of pedestrian i
        int ix =
            (int) ((ped->GetPos()._x - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
        int iy = (int) ((ped->GetPos()._y - _gridYmin) / _cellSize) + 1;

        grid[iy][ix].push_back(ped);
    }
}

void LCGrid::ClearGrid()
{
    grid.clear();
}


std::vector<Pedestrian *> LCGrid::GetNeighbourhood(const Pedestrian * ped)
{
    std::vector<Pedestrian *> neighbourhood;

    double xPed = ped->GetPos()._x;
    double yPed = ped->GetPos()._y;

    int l = (int) ((xPed - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
    int k = (int) ((yPed - _gridYmin) / _cellSize) + 1;

    std::shared_lock shared_lock(grid_mutex);
    // all neighbor cells
    for(int i = l - 1; i <= l + 1; ++i) {
        for(int j = k - 1; j <= k + 1; ++j) {
            std::copy_if(
                grid[j][i].begin(),
                grid[j][i].end(),
                std::back_inserter(neighbourhood),
                [ped](auto & other_ped) { return other_ped->GetID() != ped->GetID(); });
        }
    }

    return neighbourhood;
}
