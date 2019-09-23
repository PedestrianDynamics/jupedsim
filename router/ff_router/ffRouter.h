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
#pragma once

#include "router/Router.h"
#include "FloorfieldViaFM.h"
#include "UnivFFviaFM.h"

#include "general/Macros.h"
#include "geometry/Building.h"

class Building;
class Pedestrian;
class OutputHandler;

//log output
extern OutputHandler* Log;

/*!
 * \class FFRouter
 *
 * \brief router using floor fields to measure distances of doors, no hlines
 *
 *\ingroup Router
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
 * \author Arne Graf
 * \date Feb, 2016
 */

class FFRouter : public Router
{
public:
     /**
      * A constructor.
      *
      */
     FFRouter();
     FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals, Configuration* config);
     //FFRouter(const Building* const);

     /**
      * Destructor
      */
     virtual ~FFRouter();

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
      * \brief ReInit the router if quickest router is used. Current position of agents is considered.
      *
      * ReInit() will reconstruct the graph (nodes = doors, edges = costs) and
      * find shortest paths via Floyd-Warshall. It will reconstruct the floorfield to
      * evaluate the best doors to certain goals as they could change.
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

//     /*!
//      * \brief Sets the door that leaves the subroom in _pathsMatrix
//      *
//      * Due to the way we calculate door distances (entries in _pathsMatrix), pedestrians in a corridor
//      * tend to jump from door to door, i.e. they walk to the next door in the correct direction, but they
//      * do not traverse it. This algorithm searches for the door on the way that really leaves the subroom,
//      * and sets this door in _pathsMatrix, which in turn is needed by GetPresumableExitRoute().
//      */
//     void AvoidDoorHopping();

     /*!
      * \brief set mode (shortest, quickest, ...)
      */
      void SetMode(std::string s);
      bool MustReInit();
      void SetRecalc(double t);

    virtual void Update();

private:

protected:
     Configuration* _config;
     std::map< std::pair<int, int> , double > _distMatrix;
     std::map< std::pair<int, int> , int >    _pathsMatrix;
     //std::map< std::pair<int, int> , SubRoom* > _subroomMatrix;
     std::vector<int>                         _allDoorUIDs;
     std::vector<int>                         _localShortestSafedPeds;
     std::vector<int>                         _directionalEscalatorsUID;
     std::vector<std::pair<int, int>>         _penaltyList;
     const Building*                          _building;
     std::map<int, UnivFFviaFM*>     _locffviafm; // the actual type might be CentrePointLocalFFViaFM
     FloorfieldViaFM*                         _globalFF;
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
     bool _plzReInit = false;
     bool _hasSpecificGoals;
     bool _targetWithinSubroom;
     // If we use CentrePointDistance (i.e. CentrePointLocalFFViaFM), some algorithms can maybe be simplified
     // (AvoidDoorHopping and _subroomMatrix might be unnecessary, and some code in FindExit could go). --f.mack
     bool _useCentrePointDistance = true;
     //output filename counter: cnt
     static int _cnt;
};