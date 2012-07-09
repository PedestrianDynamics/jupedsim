/**
 * @file LCGrid.h
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version not versioned
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \brief This class Implements the Linked-Cells algorithm.

 * @section DESCRIPTION
 *   This class implements the Linked-Cells algorithm
 *   \ref{cacs.usc.edu/education/cs596/01-1LinkedListCell.pdf}
 *   A grid is laid on the complete geometry and the pedestrians are assigned the cells
 *   at each simulation step. Only pedestrians in the neighbouring cells are involved
 *   in the force computations.
 *
 *   The class is static as only one instance is needed per simulation round.
 *   This solution is fine for parallelisation as well, at least for OpenMP.
 *   TODO: investigate compliance with MPI
 *
 *
 *  Created on: Nov 16, 2010
 *
 */

#ifndef LCGRID_H_
#define LCGRID_H_


//forwarded classes
#include "../pedestrian/Pedestrian.h"

class LCGrid {

private:

	// the 'first' pedestrian in each cell
	int **pCellHead;
	//  the next pedestrians. more efficient than the double linked- list
	int *pList;
	// number of cells in x- and y-direction respectively.
	// Also to be interpreted as cell coordinates in the grid
	int pGridSizeX, pGridSizeY;
	// the cell size default to 2.2 metres
	double pCellSize;
	// rectangular area for linked cells which covers the whole geometry
	double pGrid_xmin, pGrid_xmax, pGrid_ymin, pGrid_ymax;
	// for convenience
	// will be delete in next versions
	Pedestrian** pLocalPedsCopy;

	//total number of pedestrians
	int pNpeds;

	// only pedestrians within that radius will be returned
	double pMaxEffectivDist;

public:


	LCGrid(double bounds[4], double cellsize, int nPeds);

	~LCGrid();

	/**
	 *update the cells occupation
	 */
	void Update(Pedestrian** peds);
	void Update(const vector<Pedestrian*>& peds);


	/**
	 * update this special pedestrian on the grid
	 */
	void Update(Pedestrian* ped);

	/**
	 * make a shallow copy of the initial pedestrians
	 */
	void ShallowCopy(const vector<Pedestrian*>& peds);

	/**
	 * clear the grid.
	 */
	void ClearGrid();

	/**
	 * Dump the grid, output cells and corresponding pedestrians informations
	 */
	void Dump();

	/**
	 * Dump the cells only
	 */
	void dumpCellsOnly();

	/**
	 * return a vector containing the neighbourhood of the pedestrians ped
	 * TODO: more than  10 neighbours is really too much...
	 */

	void GetNeighbourhood(const Pedestrian* ped, Pedestrian** n, int* nSize);


	void getNeighbourhood(const Point& pt, vector<Pedestrian*>& neighbourhood);

	void GetNeighbourhood(const Pedestrian* ped, vector<Pedestrian*>& neighbourhood);

};

#endif /* LGRID_H_ */
