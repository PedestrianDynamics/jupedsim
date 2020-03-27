/**
 * \file        NeighborhoodSearch.h
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
 **/
#pragma once

#include "Grid2D.h"
#include "geometry/Point.h"

#include <deque>
#include <string>
#include <vector>

//forwarded classes
class Pedestrian;
class Building;

class NeighborhoodSearch
{
private:
    double _gridXmin, _gridYmin, _cellSize;
    int _gridSizeX, _gridSizeY;

    Grid2D<std::deque<Pedestrian *>> _grid;

public:
    NeighborhoodSearch()                           = default;
    NeighborhoodSearch(const NeighborhoodSearch &) = default;
    NeighborhoodSearch(NeighborhoodSearch &&)      = default;
    NeighborhoodSearch & operator=(const NeighborhoodSearch &) = default;
    NeighborhoodSearch & operator=(NeighborhoodSearch &&) = default;

    /**
      * Constructor
      * @param boundaries the boundaries of the grid [xmin xmax ymin ymax]
      * @param cellsize the cell size
      * @param nPeds the number of pedestrians
      */
    NeighborhoodSearch(
        double gridXmin,
        double gridXmax,
        double gridYmin,
        double gridYmax,
        double cellSize);

    /**
      * Desctructor
      */
    ~NeighborhoodSearch();

    /**
      *Update the cells occupation
      */
    void Update(const std::vector<Pedestrian *> & peds);

    /**
      * Returns neighbourhood of the pedestrians ped
      * @param ped
      * @return neighbourhood
      */
    std::vector<Pedestrian *> GetNeighbourhood(const Pedestrian * ped) const;
};
