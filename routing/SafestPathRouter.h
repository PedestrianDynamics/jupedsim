/**
 * \file        SafestPathRouter.h
 * \date        Nov 29, 2013
 * \version     v0.6
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
 * This class computes the safest path for pedestrians based on information 
 * received from FDS
 *
 **/


#ifndef SAFESTPATHROUTER_H_
#define SAFESTPATHROUTER_H_
#include "AccessPoint.h"
#include "GlobalRouter.h"

class SafestPathRouter: public GlobalRouter {
public:
     SafestPathRouter();
     SafestPathRouter(int id, RoutingStrategy s);
     virtual ~SafestPathRouter();

     /**
      * @override the method from the global router.
      */
     virtual int FindExit(Pedestrian* p);

     /*
      * Load the fds simulation file and preprocess the input,
      * before calling the Init from the Global RouterEngine
      */
     virtual bool Init(Building* building);

     /**
      * Bypass using
      */
     void ComputeAndUpdateDestinations(std::vector<Pedestrian*>& pedestrians);

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
      * Print the phi index in the file
      */

     void PrintInfoToFile();


     /**
      *  reads the results from fds evac
      */
     void ReadMatrixFromFDS();
     void GetHline(Building* building);
     void UpdateMatrices();

     int GetAgentsCountInSubroom( int roomID, int subroomID);

     /**
      *
      * @param p
      */
     void UpdateRRmatrix(Pedestrian* p);

     void CalculatePhi();


     /**
      * TODO: investigate the use of a map
      */
     void MappingFloorIDtoIndex( );

     // int MapSection;



private:
     // double dMatrixPreEvac[1][11];
     // double dFinalLength[1][11];
     // last time the matrices were updated
     long int _lastUpdateTime;
     double *dFinalLineOFP;
     double *dFinalLineEvac;

     // double dFinalLineOFP[1][11];
     //     double *_finalLineEvac;
     int numberOfSubroom;
     double a;
     double b;
     double c;

     // int *preSub;
     int *flo;
     double maximalSquare;
     // double *lenthOfSection;
     // double rR[1][11];
     double *rR;

     //double peopleAtSection[1][11];
     double *peopleAtSection;
     double *squareOfSection;
     double *dFinalLength;
     double *dPeopleDensity;
     double **dPreOFP;

     FileHandler* _phiFile;
     // FileHandler* _finalLineEvac;
     // double rR[1][11];
     //double peopleAtSection[1][11];
     //double *peopleAtSection;
     // double iNt1[1][11];
     // double iNt2[1][11];
     // double iNt3[1][11];
     // double iNt4[1][11];

};

#endif /* SAFESTPATHROUTER_H_ */
