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
 * \section Description
 * This class implements the Linked-Cells algorithm
 * \ref{cacs.usc.edu/education/cs596/01-1LinkedListCell.pdf}
 * A grid is laid on the complete geometry and the pedestrians are assigned the cells
 * at each simulation step. Only pedestrians in the neighbouring cells are involved
 * in the force computations.
 *
 * The class is static as only one instance is needed per simulation round.
 * This solution is fine for parallelisation as well, at least for OpenMP.
 *
 *
 **/
#include "LCGrid.h"

#include "pedestrian/Pedestrian.h"

#include <mutex>


std::mutex grid_mutex;

//FIXME:
#define MAX_AGENT_COUNT 10000 // 1000000

LCGrid::LCGrid(double boundaries[4], double cellsize, int nPeds)
{
    _gridXmin = boundaries[0];
    _gridXmax = boundaries[1];
    _gridYmin = boundaries[2];
    _gridYmax = boundaries[3];
    _cellSize = cellsize;
    _nPeds    = nPeds + MAX_AGENT_COUNT;

    // add 1 to ensure that the whole area is covered by cells if not divisible without remainder
    _gridSizeX = (int) ((_gridXmax - _gridXmin) / _cellSize) + 1 + 2; // 1 dummy cell on each side
    _gridSizeY = (int) ((_gridYmax - _gridYmin) / _cellSize) + 1 + 2; // 1 dummy cell on each side

    // allocate memory for cells (2D-array) and initialize
    _cellHead = new int *[_gridSizeY];

    for(int i = 0; i < _gridSizeY; ++i) {
        _cellHead[i] = new int[_gridSizeX]; // nx columns

        for(int j = 0; j < _gridSizeX; ++j) {
            _cellHead[i][j] = LIST_EMPTY;
        }
    }

    // creating and resetting the pedestrians list
    _list = new int[_nPeds];
    for(int i = 0; i < _nPeds; i++)
        _list[i] = 0;

    //allocating the place for the peds copy
    _localPedsCopy = new Pedestrian *[_nPeds];
    for(int i = 0; i < _nPeds; i++)
        _localPedsCopy[i] = nullptr;
}

LCGrid::~LCGrid()
{
    delete[] _list;
    delete[] _localPedsCopy;
    for(int i = 0; i < _gridSizeY; ++i)
        delete[] _cellHead[i];
    delete[] _cellHead;
}

void LCGrid::Update(const std::vector<Pedestrian *> & peds)
{
    grid_mutex.lock();
    ClearGrid();

    for(auto & ped : peds) {
        //Pedestrian* ped = peds[p];
        int id = ped->GetID() - 1;
        // determine the cell coordinates of pedestrian i
        int ix =
            (int) ((ped->GetPos()._x - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
        int iy             = (int) ((ped->GetPos()._y - _gridYmin) / _cellSize) + 1;
        _list[id]          = _cellHead[iy][ix];
        _cellHead[iy][ix]  = id;
        _localPedsCopy[id] = ped;
    }
    grid_mutex.unlock();
}

void LCGrid::ClearGrid()
{
    // start by resetting the current list
    for(int i = 0; i < _gridSizeY; ++i) {
        for(int j = 0; j < _gridSizeX; ++j) {
            _cellHead[i][j] = LIST_EMPTY;
        }
    }

    for(int i = 0; i < _nPeds; i++) {
        _list[i]          = LIST_EMPTY;
        _localPedsCopy[i] = nullptr;
    }
}


void LCGrid::GetNeighbourhood(const Pedestrian * ped, std::vector<Pedestrian *> & neighbourhood)
{
    grid_mutex.lock();

    double xPed = ped->GetPos()._x;
    double yPed = ped->GetPos()._y;

    int l = (int) ((xPed - _gridXmin) / _cellSize) + 1; // +1 because of dummy cells
    int k = (int) ((yPed - _gridYmin) / _cellSize) + 1;

    //-1 to get  correct mapping in the array local
    int myID = ped->GetID() - 1;

    // all neighbor cells
    for(int i = l - 1; i <= l + 1; ++i) {
        for(int j = k - 1; j <= k + 1; ++j) {
            int p = _cellHead[j][i];
            // all peds in one cell
            while(p != LIST_EMPTY) {
                if(p != myID) {
                    neighbourhood.push_back(_localPedsCopy[p]);
                }
                // next ped
                p = _list[p];
            }
        }
    }

    grid_mutex.unlock();
}
