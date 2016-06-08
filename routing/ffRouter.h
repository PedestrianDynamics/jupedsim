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

#ifndef FFROUTER_H_
#define FFROUTER_H_


#include "Router.h"
#include "../general/Macros.h"
#include "../geometry/Building.h"
#include "../routing/LocalFloorfieldViaFM.h"

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
     FFRouter(int id, RoutingStrategy s, bool hasSpecificGoals);
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
      * \brief interface used by __Pedestrian__, sets (*p).exitline/.exitindex
      *
      * additional info: not available
      *
      */
     virtual int FindExit(Pedestrian* p);

     /*!
      * \brief
      *
      */
     void Reset();

     /*!
      * \brief Perform the FloydWarshall algorithm
      */
     void FloydWarshall();

     /*!
      * \brief set all the distances using ff
      */
     //void SetDistances();

     /*!
      * \brief set mode (shortest, quickest, ...)
      */
      void SetMode(std::string s);

     /*!
      * \brief notify door about time spent in that room. needed for quickest mode
      */
     void notifyDoor(Pedestrian* const p);

private:

protected:
     std::map< std::pair<int, int> , double > _distMatrix;
     std::map< std::pair<int, int> , int >    _pathsMatrix;
     std::vector<int>                         _allDoorUIDs;
     const Building*                          _building;
     std::map<int, LocalFloorfieldViaFM*>     _locffviafm;
     FloorfieldViaFM*                         _globalFF;
     std::map<int, Transition*>               _TransByUID;
     std::map<int, Transition*>               _ExitsByUID;
     std::map<int, Crossing*>                 _CroTrByUID;

     std::map<int, int>     goalToLineUIDmap; //key is the goalID and value is the UID of closest transition -> it maps goal to LineUID
     std::map<int, int>     goalToLineUIDmap2;
     std::map<int, int>     goalToLineUIDmap3;

     int _mode;
     bool _hasSpecificGoals;
     bool _targetWithinSubroom;
};

#endif /* FFROUTER_H_ */