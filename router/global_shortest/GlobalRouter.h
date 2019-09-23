/**
 * \file        GlobalRouter.h
 * \date        Dec 15, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * implement the global shortest path using the dijkstra algorithm
 *
 *
 **/
#pragma once

#include "geometry/Building.h"
#include "general/Filesystem.h"
#include "router/Router.h"

#include <cfloat>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// forwarded classes
class Pedestrian;
class AccessPoint;
class Building;
class OutputHandler;

//log output
extern OutputHandler* Log;


/*!
 * \class GlobalRouter
 *
 * \brief global router
 *
 *\ingroup Router
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * \author Ulrich Kemloh
 */
class GlobalRouter: public Router
{

public:
     /**
      * Constructor
      */
     GlobalRouter();
     GlobalRouter(int id, RoutingStrategy s);
     /**
      * Destructor
      */
     virtual ~GlobalRouter();

     virtual bool Init(Building* building);

     virtual int FindExit(Pedestrian* p);

     /**
      * Performs a check of the geometry and fixes if possible.
      * NOT IMPLEMENTED
      */
     void CheckInconsistencies();

     /**
      * write the graph as GV format to be used with graphviz
      * @param filename
      */
     void WriteGraphGV(std::string filename, int finalDestination,
               const std::vector<std::string> rooms= std::vector<std::string>());

     /**
      * Reset the routing engine and clear all pre-computed paths
      */
     void Reset();

     /**
      * Set/Get the edge cost for certain paths.
      * prefer the use of paths through floors instead of rooms
      */
     void SetEdgeCost(double cost);

     /**
      * Set/Get the edge cost for certain paths.
      * prefer the use of paths through floors instead of rooms
      */
     double GetEdgeCost() const;


protected:

     void DumpAccessPoints(int p=-1);

     /**
      * @return true if the two points are in the visibility range of each other
      * @note based on http://alienryderflex.com/intersect/
      */
     bool CanSeeEachother(const Point&pt1, const Point&pt2);

     /**
      * @obsolete
      * return a random exit
      */
     virtual int GetBestDefaultRandomExit(Pedestrian* p);

     /**
      * Generate a navigation mesh based on delauney triangulation
      */
     bool GenerateNavigationMesh();


     /**
      * Triangulate the geometry and generate the navigation lines
      */
     void TriangulateGeometry();


     /**
      *
      * @param ped the pedestrian
      * @param goalID, the goal ID.
      * @param path vector to store the intermediate destination
      */
     bool GetPath(Pedestrian* ped, int goalID, std::vector<SubRoom*>& path);

     /**
      * Populates the navigations line to cross in the vector path
      * @param ped the pedestrian
      * @param path, the vector to store
      */
     bool GetPath(Pedestrian* ped, std::vector<NavLine*>& path);

     /**
      * return the relevant aps that lead to the pedestrian final destination
      * @param ped
      */
     void GetRelevantRoutesTofinalDestination(Pedestrian *ped, std::vector<AccessPoint*>& relevantAPS);

private:
     /**
      * Compute the intermediate paths between the two given transitions IDs
      */
     void GetPath(int transID1, int transID2);

     /**
      * Perform the FloydWahrshal algorithm
      */
     void FloydWarshall();

     /**
      * Load extra routing information e.g navigation lines
      */
     bool LoadRoutingInfos(const fs::path &filename);

     /**
      * Each router is responsible of getting the correct filename
      * and doing other initializations
      */
     virtual fs::path GetRoutingInfoFile();

     /**
      * @return true if the supplied line is a wall.
      */
     bool IsWall(const Line& line, const std::vector<SubRoom*>& subrooms) const;

     /**
      * @return true if the supplied line is a Crossing.
      */
     bool IsCrossing(const Line& line, const std::vector<SubRoom*>& subrooms) const;

     /**
      * @return true if the supplied line is a Transition.
      */
     bool IsTransition(const Line& line, const std::vector<SubRoom*>& subrooms) const;

     /**
      * @return true if the supplied line is a navigation line.
      */
     bool IsHline(const Line& line, const std::vector<SubRoom*>& subrooms) const;

     /**
      * @return the minimum distance between the point and any line in the subroom.
      * This include walls,hline,crossings,transitions,obstacles
      */
     double MinDistanceToHlines(const Point& point, const SubRoom& sub);

     /**
      * @return the minimal angle in the the triangle formed by the three points
      */
     double MinAngle(const Point& p1, const Point& p2, const Point& p3);

private:
     int **_pathsMatrix;
     double **_distMatrix;
     double _edgeCost;
     //if false, the router will only return the exits and not the navigations line created through the mesh or inserted
     //via the routing file. The mesh will only be used for computing the distance.
     bool _useMeshForLocalNavigation=true;
     bool _generateNavigationMesh=false;
     //used to filter skinny edges in triangulation
     double _minDistanceBetweenTriangleEdges=-FLT_MAX;
     double _minAngleInTriangles=-FLT_MAX;
     std::vector<int> _tmpPedPath;
     std::map<int,int> _map_id_to_index;
     std::map<int,int> _map_index_to_id;
     ///map the internal crossings/transition id to
     ///the global ID (description) for that final destination
     std::map<int, int> _mapIdToFinalDestination;
     // normalize the probs
     std::default_random_engine _rdGenerator;
     std::uniform_real_distribution<double> _rdDistribution;

protected:
     // store all subrooms at the same elevation
     std::map<double, std::vector<SubRoom*> > _subroomsAtElevation;
     std::map <int, AccessPoint*> _accessPoints;
     Building *_building;

};