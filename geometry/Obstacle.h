/**
 * \file        Obstacle.h
 * \date        Jul 31, 2012
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

#include <string>
#include <vector>

//forward declarations
class Point;
class Wall;
class Line;

class Obstacle {

private:
     double _height;
     int _id;
     std::string _caption;
     std::vector<Wall> _walls;
     std::vector<Point> _poly;

public:
     /**
      * Constructor
      */
     Obstacle();

     /**
      * Destructor
      */
     virtual ~Obstacle();

     /**
      * Set/Get the obstacles' caption
      */
     std::string GetCaption() const;

     /**
      * Set/Get the obstacles' caption
      */
     void SetCaption(std::string caption);

     /**
      * Set/Get the height of the obstacle.
      * Is used for computing visibility
      */
     double GetHeight() const;

     /**
      * Set/Get the height of the obstacle.
      * Is used for computing visibility
      */
     void SetHeight(double height);

     /**
      * Set/Get the id of the obstacle
      */
     int GetId() const;

     /**
      * Set/Get the id of the obstacle
      */
     void SetId(int id);

     /**
      * construct the obstacle by adding more walls
      */
     void AddWall(const Wall& w);

     /**
      * @return All walls that constitute the obstacle
      */
     const std::vector<Wall>& GetAllWalls() const;

     /**
      * @return true if the point p is contained within the Closed Obstacle
      * @see Setclose
      */
     bool Contains(const Point& p) const;

     /**
      * Create the obstacles polygonal structure from the walls
      */
     bool ConvertLineToPoly();

     /**
      * @return the obstacle as a polygon
      */
     const std::vector<Point>&  GetPolygon() const;

     /**
      * @return the centroid of the obstacle
      */
     const Point GetCentroid() const;

     /**
      * return true if the given line intersects
      * or share common vertex with the obstacle
      */
     bool IntersectWithLine(const Line & line) const;

     /**
      * @return a nicely formatted string representation of the obstacle
      */
     std::string Write();

     /**
      * @return true if the polygon is clockwise oriented
      */
     bool IsClockwise() const;

     /**
      * @return true if the point is part of the polygon, also considering the geometry precision.
      */
     bool IsPartOfPolygon(const Point& ptw);

private:
     int WhichQuad(const Point& vertex, const Point& hitPos) const;

     // x-Koordinate der Linie von einer Eccke zur nächsten
     double Xintercept(const Point& point1, const Point& point2,
                       double hitY) const;

};
