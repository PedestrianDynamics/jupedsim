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
#pragma once

#include "geometry/Wall.h"
#include "geometry/Point.h"
#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include "router/ff_router/mesh/Trial.h"
#include "router/ff_router/mesh/RectGrid.h"

#include <vector>
#include <unordered_set>
#include <cmath>
#include <functional>

//maybe put following in macros.h
#define LOWSPEED 0.001

class TrialP
{
public:
     long int key;
     int* flag;
     double* cost;
     double* speed;
     Point* neggrad;

     TrialP() {
          key = 0;
          flag = nullptr;
          cost = nullptr;
          speed = nullptr;
          neggrad = nullptr;
     }

     TrialP(long int keyArg, double* t, double* f, int* flagArg, Point* neggradArg) {
          key = keyArg;
          cost = t;
          speed = f;
          flag = flagArg;
          neggrad = neggradArg;
     }

     ~TrialP(){}

     bool operator <(const TrialP& rhs) const
     {
          return this->cost[this->key] < rhs.cost[rhs.key];
     }

     bool operator >(const TrialP& rhs) const
     {
          return this->cost[this->key] > rhs.cost[rhs.key];
     }

     bool operator ==(const TrialP& rhs) const
     {
          return this->cost[this->key] == rhs.cost[rhs.key];
     }
};

class FloorfieldViaFM
{
public:
     FloorfieldViaFM();
     FloorfieldViaFM(const std::string&);
     FloorfieldViaFM(const Building* const buildingArg, const double hxArg, const double hyArg,
                     const double wallAvoidDistance, const bool useDistancefield, const bool onlyRoomsWithExits);
     //FloorfieldViaFM(const FloorfieldViaFM* const refFM);
     virtual ~FloorfieldViaFM();
     FloorfieldViaFM(const FloorfieldViaFM& other); //will not make a copy; only takes geometry info
     //FloorfieldViaFM& operator=(const FloorfieldViaFM& other);

     //void getDirectionAt(const Point& position, Point& direction);                                   //obsolete
     //void getDirectionToDestination (const int destID, const Point& position, Point& direction);     //obsolete
     void getDirectionToUID(int destID, const long int key, Point& direction); // shall not be used any more, therefore not virtual
     virtual void getDirectionToUID(int destID, const long int key, Point& direction, int mode);
     //void getDirectionToUIDParallel(int destID, const long int key, Point& direction);
     virtual void getDirectionToDestination (Pedestrian* ped, Point& direction);
     //void getDirectionToFinalDestination(Pedestrian* ped, Point& direction); //this is router buissness! problem in multi-storage buildings

     void createMapEntryInLineToGoalID(const int goalID);

     double getCostToDestination(const int destID, const Point& position);
     double getCostToDestination(const int destID, const Point& position, int mode);
     //double getCostToDestinationParallel(const int destID, const Point& position);

     void getDir2WallAt(const Point& position, Point& direction);
     double getDistance2WallAt(const Point& position);

     int getSubroomUIDAt(const Point& position);

     void parseBuilding(const Building* const buildingArg, const double stepSizeX, const double stepSizeY);
     void parseBuildingForExits(const Building* const buildingArg, const double stepSizeX, const double stepSizeY);
     void prepareForDistanceFieldCalculation(const bool withExits);
     template <typename T>
     void drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value);
     void setSpeed(bool useDistance2WallArg);
     void setSpeedThruPeds(Pedestrian* const* pedsArg, int nPeds, int modechoice, double radius);
     void deleteAllFFs();
     void clearAndPrepareForFloorfieldReCalc(double* costarray);
     void setNewGoalAfterTheClear(double* costarray, std::vector<Line>& GoalWallArg);
     void calculateFloorfield(std::vector<Line>& wallArg, double* costarray, Point* neggradarray);   //make private
     void calculateFloorfield(std::vector<Line>& wallArg, double* costarray, Point* neggradarray, double* speedarray);
     void calculateDistanceField(const double thresholdArg);             //make private

     void checkNeighborsAndAddToNarrowband(std::priority_queue<TrialP, std::vector<TrialP>, std::greater<TrialP> >& trialfield, TrialP key,
                                           std::function<void (TrialP)> calc);

     void calcDist2Wall(TrialP);
     void calcFloorfield(TrialP);
     //void (*checkNeighborsAndCalc)(const long int key);

     inline double onesidedCalc(double xy, double hDivF);
     inline double twosidedCalc(double x, double y, double hDivF);

     void testoutput(const char*, const char*, const double*);
     void writeFF(const std::string&, std::vector<int> targetID);
     void writeGoalFF(const std::string&, std::vector<int> targetID);

     virtual SubRoom* isInside(const long int key);
     SubRoom* GetSubroom(Pedestrian* p);

     std::map<int, int> getGoalToLineUIDmap() const
     {
          return _goalToLineUIDmap;
     }

     std::map<int, int> getGoalToLineUIDmap2() const
     {
          return _goalToLineUIDmap2;
     }

     std::map<int, int> getGoalToLineUIDmap3() const
     {
          return _goalToLineUIDmap3;
     }

     RectGrid* getGrid() const
     {
          return _grid;
     }

#ifdef TESTING
     void setGrid(RectGrid* gridArg) {_grid = gridArg;}
#endif //TESTING

protected:
     RectGrid* _grid = nullptr;
     std::vector<Line> _wall;
     std::vector<Line> _exitsFromScope;
     unsigned int _numOfExits;

     const Building* _building;

     //GridPoint Data in independant arrays (shared primary key)
     // changed to threadsafe creation when needed: int* flag;
     int* _gcode = nullptr;                 //gridcode (see Macros.h)
     SubRoom* * _subrooms = nullptr; // this is an array (first asterisk) of pointers (second asterisk)
     double* _dist2Wall = nullptr;
     double* _speedInitial = nullptr;
     double* _modifiedspeed = nullptr;
     double* _densityspeed = nullptr;
     double* _cost = nullptr;
     //long int* secKey;  //secondary key to address ... not used yet
     Point* _neggrad = nullptr; //gradients
     Point* _dirToWall = nullptr;
     // changed to threadsafe creation when needed: Trial* trialfield;

     std::map<int, double*> _goalcostmap;
     std::map<int, int>     _goalToLineUIDmap; //key is the goalID and value is the UID of closest transition -> it maps goal to LineUID
     std::map<int, int>     _goalToLineUIDmap2;
     std::map<int, int>     _goalToLineUIDmap3;
     std::map<int, Point*>  _goalneggradmap;
     std::map<int, double*> _costmap;
     std::map<int, Point*>  _neggradmap;
     // use an unordered_set for faster access (it is accessed within a critical region)
     std::unordered_set<int>  _floorfieldsBeingCalculated;
     bool maps_deleted = false; // @todo f.mack remove

     double _threshold;
     bool _useDistanceToWall;
};

// very similar to FloorfieldViaFM, but the calculation of floorfields starts at the center of the door only, not on the whole line
// this happens in order to avoid "door hopping" (for a pedestrian, it is shorter to go to a nearby door and skip half the door width)
class CentrePointFFViaFM : public virtual FloorfieldViaFM {
public:
     virtual void getDirectionToUID(int destID, const long int key, Point& direction, int mode);
};