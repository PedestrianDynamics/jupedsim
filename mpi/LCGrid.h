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

#include <vector>
#include <string>

//forwarded classes
class Pedestrian;
class Building;

class LCGrid
{

private:

     /// the 'first' pedestrian in each cell
     int **_cellHead;
     ///  the next pedestrians. more efficient than the double linked- list
     int *_list;
     /// number of cells in x- and y-direction respectively.
     /// Also to be interpreted as cell coordinates in the grid
     int _gridSizeX, _gridSizeY;
     /// the cell size default to 2.2 metres
     double _cellSize;
     /// rectangular area for linked cells which covers the whole geometry
     double _gridXmin, _gridXmax, _gridYmin, _gridYmax;
     /// for convenience
     /// will be delete in next versions
     Pedestrian** _localPedsCopy;
     ///total number of pedestrians
     int _nPeds;

public:
     /**
      * Constructor
      * @param boundaries the boundaries of the grid [xmin xmax ymin ymax]
      * @param cellsize the cell size
      * @param nPeds the number of pedestrians
      */
     LCGrid(double boundaries[4], double cellsize, int nPeds);

     /**
      * Desctructor
      */
     ~LCGrid();

     /**
      *Get pCellSize
      */
     double GetCellSize();

     /**
      *Update the cells occupation
      */
     void Update(const std::vector<Pedestrian*>& peds);

     /**
      * Update this special pedestrian on the grid
      */
     void Update(Pedestrian* ped);

     /**
      * Make a shallow copy of the initial pedestrians distribution
      */
     void ShallowCopy(const std::vector<Pedestrian*>& peds);

     /**
      * Clear the grid.
      */
     void ClearGrid();

     /**
      * Dump the content of the cells, output cells and corresponding pedestrians information
      */
     void Dump();

     /**
      * Dump the cells positions
      */
     void dumpCellsOnly();

     /**
      * Return the pedestrians in the neighborhood of a specific location
      * @param position
      * @param neighbourhood
      */
     void GetNeighbourhood(const Point& position, std::vector<Pedestrian*>& neighbourhood);

     /**
      * Returns neighbourhood of the pedestrians ped
      * @param ped
      * @param neighbourhood
      */
     void GetNeighbourhood(const Pedestrian* ped, std::vector<Pedestrian*>& neighbourhood);

     /**
      * Highlight the neighborhood of the given pedestrian
      * @param pedID
      */
     void HighlightNeighborhood(int pedID, Building* building);

     /**
      * @return an xml representation of the grid, compatible with the geometry format
      */
     std::string ToXML();

};
