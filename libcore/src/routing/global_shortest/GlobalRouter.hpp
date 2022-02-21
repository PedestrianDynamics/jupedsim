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

#include "geometry/Building.hpp"
#include "routing/GlobalRouterParameters.hpp"
#include "routing/Router.hpp"

#include <cfloat>
#include <string>
#include <vector>

// forwarded classes
class Pedestrian;
class AccessPoint;
class Building;
class OutputHandler;


class GlobalRouter : public Router
{
public:
    GlobalRouter(Building * building, const GlobalRouterParameters & parameters);
    ~GlobalRouter() override;

    int FindExit(Pedestrian * p) override;

    void Update() override{};

    /**
      * Reset the routing engine and clear all pre-computed paths
      */
    void Reset();

    /**
      * @obsolete
      * return a random exit
      */
    int GetBestDefaultRandomExit(Pedestrian * p);

    /**
      * Triangulate the geometry and generate the navigation lines
      */
    void TriangulateGeometry();

    /**
      * Populates the navigations line to cross in the vector path
      * @param ped the pedestrian
      * @param path, the vector to store
      */
    bool GetPath(Pedestrian * ped, std::vector<Line *> & path);

    /**
      * return the relevant aps that lead to the pedestrian final destination
      * @param ped
      */
    void
    GetRelevantRoutesTofinalDestination(Pedestrian * ped, std::vector<AccessPoint *> & relevantAPS);

private:
    bool init();
    /**
      * Compute the intermediate paths between the two given transitions IDs
      */
    void GetPath(int transID1, int transID2);

    /**
      * Perform the FloydWahrshal algorithm
      */
    void FloydWarshall();

    /**
      * @return true if the supplied line is a wall.
      */
    bool IsWall(const Line & line, const std::vector<SubRoom *> & subrooms) const;

    /**
      * @return true if the supplied line is a Crossing.
      */
    bool IsCrossing(const Line & line, const std::vector<SubRoom *> & subrooms) const;

    /**
      * @return true if the supplied line is a Transition.
      */
    bool IsTransition(const Line & line, const std::vector<SubRoom *> & subrooms) const;

    /**
      * @return true if the supplied line is a navigation line.
      */
    bool IsHline(const Line & line, const std::vector<SubRoom *> & subrooms) const;

    /**
      * @return the minimum distance between the point and any line in the subroom.
      * This include walls,hline,crossings,transitions,obstacles
      */
    double MinDistanceToHlines(const Point & point, const SubRoom & sub);

    /**
      * @return the minimal angle in the the triangle formed by the three points
      */
    double MinAngle(const Point & p1, const Point & p2, const Point & p3);

private:
    int ** _pathsMatrix;
    double ** _distMatrix;
    double _edgeCost;
    int _exitsCnt;
    //if false, the router will only return the exits and not the navigations line created through the mesh or inserted
    //via the routing file. The mesh will only be used for computing the distance.
    bool _useMeshForLocalNavigation = true;
    bool _generateNavigationMesh    = false;
    //used to filter skinny edges in triangulation
    double _minDistanceBetweenTriangleEdges = -FLT_MAX;
    double _minAngleInTriangles             = -FLT_MAX;
    std::vector<int> _tmpPedPath;
    std::map<int, int> _map_id_to_index;
    std::map<int, int> _map_index_to_id;
    ///map the internal crossings/transition id to
    ///the global ID (description) for that final destination
    std::map<int, int> _mapIdToFinalDestination;

    // store all subrooms at the same elevation
    std::map<double, std::vector<SubRoom *>> _subroomsAtElevation;
    std::map<int, AccessPoint *> _accessPoints;
    Building * _building;
};
