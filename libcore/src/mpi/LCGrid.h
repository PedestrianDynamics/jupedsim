/**
 * \file        LCGrid.h
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
 * The class is static as only one instance is needed per simulation round.
 * This solution is fine for parallelisation as well, at least for OpenMP.
 *
 *
 **/
#pragma once

#include "geometry/Point.h"

#include <deque>
#include <string>
#include <vector>

//forwarded classes
class Pedestrian;
class Building;

class LCGrid
{
private:
    /// rectangular area for linked cells which covers the whole geometry
    double _gridXmin, _gridYmin;
    double _cellSize;


    /// number of cells in x- and y-direction respectively.
    /// Also to be interpreted as cell coordinates in the grid
    int _gridSizeX, _gridSizeY;

    using grid_elem_t = std::deque<Pedestrian *>;

    std::vector<std::vector<grid_elem_t>> grid;

    /**
      * Clear the grid.
      */
    void ClearGrid();

public:
    /**
      * Constructor
      * @param boundaries the boundaries of the grid [xmin xmax ymin ymax]
      * @param cellsize the cell size
      * @param nPeds the number of pedestrians
      */
    LCGrid(double gridXmin, double gridXmax, double gridYmin, double gridYmax, double cellSize);

    /**
      * Desctructor
      */
    ~LCGrid();

    /**
      *Update the cells occupation
      */
    void Update(const std::vector<Pedestrian *> & peds);

    /**
      * Returns neighbourhood of the pedestrians ped
      * @param ped
      * @param neighbourhood
      */
    void GetNeighbourhood(const Pedestrian * ped, std::vector<Pedestrian *> & neighbourhood);
};
