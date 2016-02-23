/**
 * \file        FloorfieldViaFM.h
 * \date        Mar 05, 2015
 * \version     N/A (v0.6)
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
 * Implementation of classes for ...
 *
 *
 **/

 //remark:
 //refac the code to use enums instead of integer values where integer values code sth
 //was considered, but enum classes do not implicitly cast to int
 //rather use makros/masks like in plain C? or just makros (defines)?
 //this would make it easier to read

#ifndef FLOORFIELDVIAFM_H
#define FLOORFIELDVIAFM_H

#include <vector>
#include <cmath>
#include <functional>
#include "mesh/RectGrid.h"
#include "../geometry/Wall.h"
#include "../geometry/Point.h"
#include "../geometry/Building.h"
#include "../geometry/SubRoom.h" //check: should Room.h include SubRoom.h??
#include "../routing/mesh/Trial.h"
#include "../pedestrian/Pedestrian.h"

//maybe put following in macros.h
#define LOWSPEED 0.001

class FloorfieldViaFM
{
public:
     FloorfieldViaFM();
     FloorfieldViaFM(const std::string&);
     FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg, const double wallAvoidDistance, const bool useDistancefield, const std::string&);
     virtual ~FloorfieldViaFM();
     FloorfieldViaFM(const FloorfieldViaFM& other);
     //FloorfieldViaFM& operator=(const FloorfieldViaFM& other);

     void getDirectionAt(const Point& position, Point& direction);                                   //obsolete
     //void getDirectionToDestination (const int destID, const Point& position, Point& direction);     //obsolete
     void getDirectionToUID(int destID, const long int key, Point& direction);
     void getDirectionToDestination (Pedestrian* ped, Point& direction);
     void getDirectionToFinalDestination(Pedestrian* ped, Point& direction);
     void getDirectionToGoalID(const int goalID);
     double getCostToDestination(const int destID, const Point& position);
     void getDir2WallAt(const Point& position, Point& direction);
     double getDistance2WallAt(const Point& position);

     void parseBuilding(const Building* const buildingArg, const double stepSizeX, const double stepSizeY);
     void prepareForDistanceFieldCalculation(std::vector<Line>& wallArg);
     void drawLinesOnGrid(std::vector<Line>& wallArg, double* const target, const double outside);
     void setSpeed(bool useDistance2Wall);
     void clearAndPrepareForFloorfieldReCalc(double* costarray);
     void setNewGoalAfterTheClear(double* costarray, std::vector<Line>& GoalWallArg);
     void calculateFloorfield(double* costarray, Point* neggradarray);   //make private
     void calculateDistanceField(const double thresholdArg);             //make private

     void checkNeighborsAndAddToNarrowband(Trial* &smallest, Trial* &biggest, const long int key, std::function<void (const long int)> checkNeighborsAndCalc);

     void checkNeighborsAndCalcDist2Wall(const long int key);
     void checkNeighborsAndCalcFloorfield(const long int key);
     //void (*checkNeighborsAndCalc)(const long int key);

     inline double onesidedCalc(double xy, double hDivF);
     inline double twosidedCalc(double x, double y, double hDivF);

     void testoutput(const char*, const char*, const double*);
     void writeFF(const std::string&);

#ifdef TESTING
     void setGrid(RectGrid* gridArg) {grid = gridArg;}
     Trial* getTrial() {return trialfield;}
#endif //TESTING

protected:
     RectGrid* grid;
     std::vector<Line> wall;
     int numOfExits;

     const Building* building;

     //stuff to handle wrapper grid (unused, cause RectGrid handles offset)
     double offsetX;
     double offsetY;

     //GridPoint Data in independant arrays (shared primary key)
     int* flag;                  //flag:( 0 = unknown, 1 = singel, 2 = double, 3 = final, 4 = added to trial but not calculated, -7 = outside)
     double* dist2Wall;
     double* speedInitial;
     double* modifiedspeed;
     double* cost;
     long int* secKey;  //secondary key to address ... not used yet
     Point* neggrad; //gradients
     Point* dirToWall;
     Trial* trialfield;

     std::map<int, double*> goalcostmap;
     std::map<int, int>     goalToLineUIDmap; //key is the goalID and value is the UID of closest transition -> it maps goal to LineUID
     std::map<int, int>     goalToLineUIDmap2;
     std::map<int, int>     goalToLineUIDmap3;
     std::map<int, Point*>  goalneggradmap;
     std::map<int, double*> costmap;
     std::map<int, Point*>  neggradmap;

     double threshold;
};

#endif // FLOORFIELDVIAFM_H
