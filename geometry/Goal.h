/**
 * \file        Goal.h
 * \date        Sep 12, 2013
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
 *
 *
 **/
#pragma once

#include "Point.h"

#include <boost/polygon/polygon.hpp>
#include <boost/geometry.hpp>

#include <string>
#include <vector>

//forward declarations
class Wall;
class Point;
class Crossing;
class Pedestrian;

namespace bg = boost::geometry;
typedef bg::model::polygon<Point, false, false> polygon_type;

class Goal {

protected:
     int _isFinalGoal;
     int _id;
     int _roomID;
     int _subRoomID;

    Point _centroid;
    std::string _caption;
    std::vector<Wall> _walls;
    std::vector<Point> _poly;
    Crossing* _crossing;

    polygon_type _boostPoly;
    bool _inside;

public:



public:
     Goal();
     virtual ~Goal();

     /**
      * Set/Get the obstacles' caption
      */
     std::string GetCaption() const;

     /**
      * Set/Get the obstacles' caption
      */
     void SetCaption(std::string caption);

     /**
      * Set/Get the id of the Goal
      */
     int GetId() const;

     /**
      * Set/Get the id of the Goal
      */
     void SetId(int id);

     /**
      * construct the Goal by adding more walls
      */
     void AddWall(const Wall& w);

     /**
      * @return All walls that constitute the Goal
      */
     const std::vector<Wall>& GetAllWalls() const;

     /**
      * @return true if the point p is contained within the Closed Goal
      */
     bool Contains(const Point& p) const;

     /**
      * Create the obstacles polygonal structure from the walls
      */
     bool ConvertLineToPoly();

     /**
      * @return the Goal as a polygon
      */
     const std::vector<Point>&  GetPolygon() const;

     /**
      * agents are remove from the simulation when they reached a final goal
      */
     int GetIsFinalGoal() const;

     /**
      * agents are remove from the simulation when they reached a final goal
      */
     void SetIsFinalGoal(int isFinalGoal);

     /**
      * @return the centroid of the subroom
      * @see http://en.wikipedia.org/wiki/Centroid
      */
     void ComputeCentroid() ;

     /**
      * @return the centroid of the goal
      * @see ComputeControid
      */
     const Point& GetCentroid() const;

     /**
      * @return a nicely formatted string representation of the Goal
      */
     std::string Write();

     Crossing* GetCentreCrossing();

     bool IsInsideGoal(Pedestrian* ped) const;


    int GetRoomID() const;

    void SetRoomID(int _roomID);

    int GetSubRoomID() const;

    void SetSubRoomID(int _subRoomID);

    bool IsInsideGoal(const Point& point) const;

private:
    bool IsClockwise();
    bool CreateBoostPoly();

    int WhichQuad(const Point& vertex, const Point& hitPos) const;

     // x-Koordinate der Linie von einer Eccke zur nächsten
     double Xintercept(const Point& point1, const Point& point2,
                       double hitY) const;

};
