/**
 * \file        cdt.h
 * \date        Jul 4, 2014
 * \version     v0.5
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
 *
 *
 **/


#ifndef CDT_H
#define CDT_H

#include "advancing_front.h"
#include "sweep_context.h"
#include "sweep.h"

/**
 *
 * @author Mason Green <mason.green@gmail.com>
 *
 */

namespace p2t {

class CDT {
public:

     /**
      * Constructor - add polyline with non repeating points
      *
      * @param polyline
      */
     CDT(std::vector<Point*> polyline);

     /**
      * Destructor - clean up memory
      */
     ~CDT();

     /**
      * Add a hole
      *
      * @param polyline
      */
     void AddHole(std::vector<Point*> polyline);

     /**
      * Add a steiner point
      *
      * @param point
      */
     void AddPoint(Point* point);

     /**
      * Triangulate - do this AFTER you've added the polyline, holes, and Steiner points
      */
     void Triangulate();

     /**
      * Get CDT triangles
      */
     std::vector<Triangle*> GetTriangles();

     /**
      * Get triangle map
      */
     std::list<Triangle*> GetMap();

private:

     /**
      * Internals
      */

     SweepContext* sweep_context_;
     Sweep* sweep_;

};

}

#endif
