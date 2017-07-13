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
#ifndef JPSCORE_UNIVFFVIAFM_H
#define JPSCORE_UNIVFFVIAFM_H

#include <string>
#include <vector>
#include <map>
#include <float.h>
#include "../../general/Macros.h"

class Room;
class SubRoom;
class Building;
class Configuration;
class Point;
class RectGrid;
class Line;

enum SPEEDFIELD {        //this enum is used as index in _speedFieldSelector
     INITIAL_SPEED=0,    //homogen speed in walkable area, nealy zero in walls
     REDU_WALL_SPEED=1,   //reduced wall speed
     PED_SPEED=2         //standing agents reduce speed, so that jams will be considered in ff
};

enum TARGETMODE {
     LINESEGMENT=0,
     CENTERPOINT
};

enum USERMODE {
     DISTANCE_MEASUREMENTS_ONLY,
     DISTANCE_AND_DIRECTIONS_USED
};

class CompareCost {      //this class is used in std::priority_queue in UnivFFviaFM::calcFF
public:
     CompareCost(double* costarray) : _costarray(costarray) {}
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
     UnivFFviaFM(Room *a, const Configuration *b, double hx, double wallAvoid, bool useWallAvoid);
     UnivFFviaFM(Room *a, const Configuration *b, double hx, double wallAvoid, bool useWallAvoid, std::vector<int> wantedDoors);
     UnivFFviaFM(SubRoom *sr, const Configuration *conf, double hx, double wallAvoid, bool useWallAvoid);
     UnivFFviaFM(SubRoom *subRoomArg, const Configuration *confArg, double hx, double wallAvoid, bool useWallAvoid, std::vector<int> wantedDoors);
     void create(std::vector<Line>& walls, std::map<int, Line>& doors, std::vector<int> targetUIDs, int mode,
                 double spacing, double wallAvoidDist, bool useWallAvoid);
     UnivFFviaFM() {};
     UnivFFviaFM(UnivFFviaFM&){};
     ~UnivFFviaFM(){};

     void addTarget(const int uid, Line* door);
     void addTarget(const int uid);
     void addAllTargets();
     std::vector<int> getKnownDoorUIDs();

     double getCostToDestination(const int destID, const Point& position, int mode) {return 0.;};
     double getCostToDestination(const int destID, const Point& position) {return 0.;};
     RectGrid* getGrid(){return nullptr;};
     virtual void getDirectionToUID(int destID, const long int key, Point& direction, int mode){};
     void getDirectionToUID(int destID, const long int key, Point& direction){};
     void writeFF(const std::string&, std::vector<int> targetID);

     void createRectGrid(std::vector<Line>& walls, std::map<int, Line>& doors, double spacing);
     void processGeometry(std::vector<Line>&walls, std::map<int, Line>& doors);

     void drawLinesOnGrid(std::map<int, Line>& doors, int *const grid);
     template <typename T>
     void drawLinesOnGrid(std::vector<Line>& wallArg, T* const target, const T value);
     template <typename T>
     void drawLinesOnGrid(Line& line, T* const target, const T value);

     void calcFF(double*, Point*, const double* const);
     void calcCost(const long int key, double* cost, Point* dir, const double* const speed);
     inline double onesidedCalc(double xy, double hDivF);
     inline double twosidedCalc(double x, double y, double hDivF);

private:
     Building* _building = nullptr;
     Configuration* _configuration = nullptr;
     int _mode = LINESEGMENT;                     //default
     int _user = DISTANCE_AND_DIRECTIONS_USED;    //default
     int _speedmode = FF_HOMO_SPEED;              //default
     RectGrid* _grid = nullptr;
     long int _nPoints = 0;
     std::vector<double*> _speedFieldSelector;
     int* _gridCode = nullptr;

     double _wallAvoidDistance = 0.;
     bool _useWallAvoidance = false;

     //the following maps are responsible for dealloc the arrays
     std::map<int, double*> _costFieldWithKey;
     std::map<int, Point*> _directionFieldWithKey;
     std::vector<int> _uids;
     std::map<int, Line> _doors;

};


#endif //JPSCORE_UNIVFFVIAFM_H
