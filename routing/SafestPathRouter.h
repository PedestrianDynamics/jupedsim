/**
 * @file    SafestPathRouter.h
 * Created on: 29.11.2013
 * Copyright (C) <2009-2013>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *  This class computes the safest path for pedestrians based on information
 * received from FDS
 *
 *
 */

#ifndef SAFESTPATHROUTER_H_
#define SAFESTPATHROUTER_H_

#include "GlobalRouter.h"

class SafestPathRouter: public GlobalRouter {
public:
	SafestPathRouter();
	virtual ~SafestPathRouter();

	/**
	 * @override the method from the global router.
	 */
	virtual int FindExit(Pedestrian* p);

	/*
	 * Load the fds simulation file and preprocess the input,
	 * before calling the Init from the Global RouterEngine
	 */
	virtual void Init(Building* building);

private:
	/**
	 * Compute the safest path for the given pedestrian and update the destination.
	 * FindExit from the global router is called afterward to perform the navigation
	 * ONLY the final destination should be updated in this function, as the navigation
	 * itself is handled by the global router engine
	 * @return the new safest goalID, -1, if there was an Error
	 */
	int ComputeSafestPath(Pedestrian* p);


	/**
	 * do some initialisation stuff...
	 */
	void Initialize();


	/**
	 *  reads the results from fds evac
	 */
	void ReadMatrixFromFDS();



	int GetAgentsCountInSubroom( int roomID, int subroomID);
	void main_1(Pedestrian* p);
	void main_2( );
	void main_3(Pedestrian* p);

	int MapSection;



private:
	// double dMatrixPreEvac[1][11];
	// double dFinalLength[1][11];
	// double dFinalLineOFP[1][11];
	 double dFinalLineEvac[1][11];
//	 double *_finalLineEvac;

	// double rR[1][11];
	 double **rR;

	 //double peopleAtSection[1][11];
	 double *peopleAtSection;
	// double iNt1[1][11];
	// double iNt2[1][11];
	// double iNt3[1][11];
	// double iNt4[1][11];
	 //double fI[11][11];
};

#endif /* SAFESTPATHROUTER_H_ */
