//
// Created by arne on 5/9/17.
//
/**
 * \file        UnivFFviaFM.h
 * \date        May 09, 2017
 * \version     N/A (v0.8.x)
 * \copyright   <2017-2020> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Implementation of classes for a reworked floorfield. A floorfield in general
 * yields a cost field to a specific goal and a correlated vectorfield for the
 * optimal path in terms of the cost value.
 *
 * Rework focused on a cleaner structure and less inheritance (no diamond) and
 * less workarounds.
 *
 *
 **/
#pragma once

#include "general/Filesystem.h"
#include "general/Macros.h"

#include <string>
#include <vector>
#include <map>
#include <float.h>

class Pedestrian;
class Room;
class SubRoom;
class Building;
class Configuration;
class Point;
class RectGrid;
class Line;

//enum SPEEDFIELD {        //this enum is used as index in _speedFieldSelector
//     INITIAL_SPEED=0,    //homogen speed in walkable area, nealy zero in walls
//     REDU_WALL_SPEED=1,   //reduced wall speed
//     PED_SPEED=2         //standing agents reduce speed, so that jams will be considered in ff
//};
//
//enum TARGETMODE {
//     LINESEGMENT=0,
//     CENTERPOINT
//};
//
//enum USERMODE {
//     DISTANCE_MEASUREMENTS_ONLY,
//     DISTANCE_AND_DIRECTIONS_USED
//};

class CompareCostTrips {      //this class is used in std::priority_queue in UnivFFviaFM::calcFF
public:
     CompareCostTrips(double* costarray) : _costarray(costarray) {}
     bool operator() (const int a, const int b) const {
          return _costarray[a] > _costarray[b];
     }

private:
     double* _costarray = nullptr;
};

class UnivFFviaFM {
public:
     UnivFFviaFM(Room* a, Building* b, double c, double e, bool f);
     UnivFFviaFM(SubRoom* a, Building* b, double c, double e, bool f);
     UnivFFviaFM(Room* a, Configuration* const b, double hx, double wallAvoid, bool useWallDistances);
     UnivFFviaFM(Room* a, Configuration* const b, double hx, double wallAvoid, bool useWallDistances, std::vector<int> wantedDoors);
     UnivFFviaFM(SubRoom* sr, Configuration* const conf, double hx, double wallAvoid, bool useWallDistances);
     UnivFFviaFM(SubRoom* subRoomArg, Configuration* const confArg, double hx, double wallAvoid, bool useWallDistances, std::vector<int> wantedDoors);
     void create(std::vector<Line>& walls, std::map<int, Line>& doors, std::vector<int> targetUIDs, int mode,
                 double spacing, double wallAvoidDist, bool useWallDistances);
     void recreateAllForQuickest();
     UnivFFviaFM() {};
     UnivFFviaFM(UnivFFviaFM&){};
     virtual ~UnivFFviaFM();

     void addTarget(const int uid, Line* door, double* costarray = nullptr, Point* gradarray = nullptr);
     void addTarget(const int uid, double* costarray = nullptr, Point* gradarray = nullptr);
     void addAllTargets();
     void addAllTargetsParallel();
     void addTargetsParallel(std::vector<int> wantedDoors);
     std::vector<int> getKnownDoorUIDs();
     void setUser(int userArg);
     void setMode(int modeArg);
     void setSpeedMode(int speedModeArg);
     SubRoom** getSubRoomFF();
     SubRoom* getSubRoom(const Point& pos);

     double getCostToDestination(const int destID, const Point& position, int mode);
     double getCostToDestination(const int destID, const Point& position);
     double getDistanceBetweenDoors(const int door1_ID, const int door2_ID);
     RectGrid* getGrid();
     virtual void getDirectionToUID(int destID, long int key, Point& direction, int mode);
     void getDirectionToUID(int destID, long int key, Point& direction);
     virtual void getDirectionToUID(int destID, const Point& pos, Point& direction, int mode);
     void getDirectionToUID(int destID, const Point& pos, Point& direction);
     double getDistance2WallAt(const Point& pos);
     void getDir2WallAt(const Point& pos, Point& p);

     void writeFF(const fs::path& filename, std::vector<int> targetID);

     void createRectGrid(std::vector<Line>& walls, std::map<int, Line>& doors, double spacing);
     void processGeometry(std::vector<Line>&walls, std::map<int, Line>& doors);
     void markSubroom(const Point& insidePoint, SubRoom* const value);
     void createReduWallSpeed(double* reduWallSpeed);
     void createPedSpeed(Pedestrian* const * pedsArg, int nsize, int modechoice, double radius);
     void finalizeTargetLine(const int uid, const Line& tempTargetLine, Point* newArrayPt, Point& passvector);

     void drawLinesOnGrid(std::map<int, Line>& doors, int *const grid);
     template <typename T>
     void drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value);
     template <typename T>
     void drawLinesOnGrid(Line& line, T* const target, const T value);

     template <typename T>
     void drawLinesOnWall(std::vector<Line>& wallArg, T* const target, const T value);
     template <typename T>
     void drawLinesOnWall(Line& line, T* const target, const T value);

     void calcFF(double*, Point*, const double* const);
     void calcCost(const long int key, double* cost, Point* dir, const double* const speed);
     void calcDF(double*, Point*, const double* const);
     void calcDist(const long int key, double* cost, Point* dir, const double* const speed);
     inline double onesidedCalc(double xy, double hDivF);
     inline double twosidedCalc(double x, double y, double hDivF);

private:
     Building* _building = nullptr;
     Configuration* _configuration = nullptr;
     int _room = -1;                              //not set
     int _mode = LINESEGMENT;                     //default
     int _user = DISTANCE_AND_DIRECTIONS_USED;    //default
     int _speedmode = FF_HOMO_SPEED;              //default
     int _scope = 0;                              //not set / unknown
     bool _directCalculation = true;
     RectGrid* _grid = nullptr;
     long int _nPoints = 0;
     std::vector<double*> _speedFieldSelector;
     int* _gridCode = nullptr;
     SubRoom* * _subrooms = nullptr; // this is an array (first asterisk) of pointers (second asterisk)

     double _wallAvoidDistance = 0.;
     bool _useWallDistances = false;    //could be used in DirectionStrategy even if mode _speedmode is FF_HOMO_SPEED

     //the following maps are responsible for dealloc the arrays
     std::map<int, double*> _costFieldWithKey;
     std::map<int, Point*> _directionFieldWithKey;

     std::vector<int> _uids;
     std::map<int, Line> _doors;
     std::vector<int> _toDo;

     std::map<int, Point> _subroomUIDtoInsidePoint;
     std::map<int, SubRoom*> _subroomUIDtoSubRoomPtr;
     std::map<SubRoom*, Point> _subRoomPtrTOinsidePoint;

};