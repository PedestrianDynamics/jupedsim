/**
 * \file        LCGrid.h
 * \date        Nov 16, 2010
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#ifndef LCGRID_H_
#define LCGRID_H_

#include <vector>
#include "../geometry/Point.h"

//forwarded classes
class Pedestrian;


class LCGrid {

private:

     /// the 'first' pedestrian in each cell
     int **pCellHead;
     ///  the next pedestrians. more efficient than the double linked- list
     int *pList;
     /// number of cells in x- and y-direction respectively.
     /// Also to be interpreted as cell coordinates in the grid
     int pGridSizeX, pGridSizeY;
     /// the cell size default to 2.2 metres
     double pCellSize;
     /// rectangular area for linked cells which covers the whole geometry
     double pGrid_xmin, pGrid_xmax, pGrid_ymin, pGrid_ymax;
     /// for convenience
     /// will be delete in next versions
     Pedestrian** pLocalPedsCopy;

     ///total number of pedestrians
     int pNpeds;


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
      *Update the cells occupation
      */

     void Update(Pedestrian** peds);
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
      * Return a vector containing the neighbourhood of the pedestrians ped
      * @param ped the considered pedestrian
      * @param neighborhood an array containing the pedestrians
      * @param nSize the number of returned pedestrians
      */
     void GetNeighbourhood(const Pedestrian* ped, Pedestrian** neighborhood, int* nSize);


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

};

#endif /* LGRID_H_ */
