//
// Created by arne on 3/29/17.
//
/**
 * \file        ffRouter.h
 * \date        Feb 19, 2016
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 * The Floorfield Kit is the rework of the prototype FloorfieldViaFM - class.
 * It will restructur the former version to avoid several downsides (diamond
 * inheritance, line/centerpoint version (DRY), overdoing the precalc, ...)
 * We try to build it in a cleaner, easier version.
 *
 * A floorfield will be calculated and stored. The user can access distances(cost)
 * values as well as the gradient of that.
 *
 * Cost Values are calculated via the fastmarching algorithm. Other algorithms might
 * follow, if they provide better abilities to work in paralell.
 *
 * The new grid-code/subroom version should be reused for all floorfields, that work
 * on the same part of the geometry (room/subroom e.g. floor/room)
 *
 * old ffrouter description:
 * This router is an update of the former Router.{cpp, h} - Global-, Quickest
 * Router System. In the __former__ version, a graph was created with doors and
 * hlines as nodes and the distances of (doors, hlines), connected with a line-
 * of-sight, was used as edge-costs. If there was no line-of-sight, there was no
 * connecting edge. On the resulting graph, the Floyd-Warshall algorithm was
 * used to find any paths. In the "quickest-___" variants, the edge cost was not
 * determined by the distance, but by the distance multiplied by a speed-
 * estimate, to find the path with minimum travel times. This whole construct
 * worked pretty well, but dependend on hlines to create paths with line-of-
 * sights to the next target (hline/door).
 *
 * In the ffRouter, we want to overcome hlines by using floor fields to
 * determine the distances. A line of sight is not required any more. We hope to
 * reduce the graph complexity and the preparation-needs for new geometries.
 *
 * To find a counterpart for the "quickest-____" router, we can either use
 * __special__ floor fields, that respect the travel time in the input-speed map,
 * or take the distance-floor field and multiply it by a speed-estimate (analog
 * to the former construct.
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * Questions to solve: how to deal with goalID == doorID problem in matrix
 *
 * Restrictions/Requirements: Floorfields are not really 3D supporting:
 *
 * A room may not consist of subrooms which overlap in their projection (onto
 * x-y-plane). So subrooms, that are positioned on top of others (in stairways
 * for example), must be separated into different rooms.
 *
 * floorfields do not consider z-coordinates. Distances of two grid points are
 * functions of (x, y) and not (x, y, z). Any slope will be neglected.
 *
 **/

#ifndef JPSCORE_FFKIT_H
#define JPSCORE_FFKIT_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <functional>
#include "mesh/RectGrid.h"
#include "../../geometry/Wall.h"
#include "../../geometry/Point.h"
#include "../../geometry/Building.h"
#include "../../geometry/SubRoom.h" //check: should Room.h include SubRoom.h??
#include "./mesh/Trial.h"
#include "../../pedestrian/Pedestrian.h"

//class Building;
//class Pedestrian;
class OutputHandler;

//log output
extern OutputHandler* Log;

class MapRouter : public Router
{
public:
     /**
      * A constructor.
      *
      */
     MapRouter();
     MapRouter(int id, RoutingStrategy s, bool hasSpecificGoals, Configuration* config);


     /**
      * Destructor
      */
     virtual ~MapRouter();

     /*!
      * \brief Init the router (must be called before use)
      *
      * Init() will construct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It needs the floor fields
      *
      *
      * \param[in] [name of input parameter] [its description]
      * \param[out] [name of output parameter] [its description]
      * \return [information about return value]
      * \sa [see also section]
      * \note [any note about the function you might have]
      * \warning [any warning if necessary]
      */
     virtual bool Init(Building* building);

     /*!
      * \brief Init the router (must be called before use)
      *
      * Init() will construct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It needs the floor fields
      *
      *
      * \param[in] [name of input parameter] [its description]
      * \param[out] [name of output parameter] [its description]
      * \return [information about return value]
      * \sa [see also section]
      * \note [any note about the function you might have]
      * \warning [any warning if necessary]
      */
     virtual bool Init(Configuration* conf);

     /*!
      * \brief ReInit the router (must be called after each event (open/close change)
      *
      * ReInit() will reconstruct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It will reconstruct the floorfield to
      * evaluate the best doors to certain goals as they could change. Further on it
      * will take the information of former floorfields, if useful.
      *
      *
      * \param[in] [name of input parameter] [its description]
      * \param[out] [name of output parameter] [its description]
      * \return [information about return value]
      * \sa [see also section]
      * \note [any note about the function you might have]
      * \warning [any warning if necessary]
      */
     virtual bool ReInit();

     /*!
      * \brief interface used by __Pedestrian__, sets (*p).exitline/.exitindex
      *
      * additional info: not available
      *
      */
     virtual int FindExit(Pedestrian* p);

     /*!
      * \brief Perform the FloydWarshall algorithm
      */
     void FloydWarshall();

     /*!
      * \brief set mode (shortest, quickest, ...)
      */
     void SetMode(std::string s);

     /*!
      * \brief mark pedestrian as not being in the first room anymore and return to normal routing
      */
     void save(Pedestrian* const p);

     /*!
      * \brief Get the route the pedestrian p wants to take (according to _pathsMatrix)
      * @param p The pedestrian in question
      * @return A set containing (subroom*, doorUID) pairs. The floorfields needed are inside the subroom, originating from the door.
      */
     std::set<std::pair<SubRoom*, int>> GetPresumableExitRoute(Pedestrian* p);

private:

protected:
     Configuration* _config;
     std::map< std::pair<int, int> , double > _distMatrix;
     std::map< std::pair<int, int> , int >    _pathsMatrix;
     std::map< std::pair<int, int> , SubRoom* > _subroomMatrix;
     std::vector<int>                         _allDoorUIDs;
     std::vector<int>                         _localShortestSafedPeds;
     const Building*                          _building;
     std::map<int, UnivFloorfield*>     _locffviafm; // the actual type might be CentrePointLocalFFViaFM
     UnivFloorfield*                         _globalFF;
     std::map<int, Transition*>               _TransByUID;
     std::map<int, Transition*>               _ExitsByUID;
     std::map<int, Crossing*>                 _CroTrByUID;

     std::map<int, int>     _goalToLineUIDmap; //key is the goalID and value is the UID of closest transition -> it maps goal to LineUID
     std::map<int, int>     _goalToLineUIDmap2;
     std::map<int, int>     _goalToLineUIDmap3;
     std::map<int, int>     _finalDoors; // _finalDoors[i] the UID of the last door the pedestrian with ID i wants to walk through

     int _mode;
     double _timeToRecalc = 0.;
     double _recalc_interval;
     bool _hasSpecificGoals;
     bool _targetWithinSubroom;
     // If we use CentrePointDistance (i.e. CentrePointLocalFFViaFM), some algorithms can maybe be simplified
     // (AvoidDoorHopping and _subroomMatrix might be unnecessary, and some code in FindExit could go). --f.mack
     bool _useCentrePointDistance = true;
     //output filename counter: cnt
     static int _cnt;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

class UnivFloorfield
{
public:
     UnivFloorfield();
     UnivFloorfield(const std::string&);
     UnivFloorfield(const Building* const buildingArg, const double hxArg, const double hyArg,
                     const double wallAvoidDistance, const bool useDistancefield, const bool onlyRoomsWithExits);
     //UnivFloorfield(const UnivFloorfield* const refFM);
     virtual ~UnivFloorfield();
     UnivFloorfield(const UnivFloorfield& other); //will not make a copy; only takes geometry info
     //UnivFloorfield& operator=(const UnivFloorfield& other);

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

     void checkNeighborsAndAddToNarrowband(std::priority_queue<TrialP, std::vector<TrialP>, std::greater<TrialP>>& trialfield, TrialP key,
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


#endif //JPSCORE_FFKIT_H
