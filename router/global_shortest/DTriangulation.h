/**
 * \file        DTriangulation.h
 * \date        Nov 30, 2012
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
 * Perform the Delauney triangulation of a polygon with holes.
 *
 *
 **/
#pragma once


#include <poly2tri.h>

#include <cstdlib>
#include <time.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>

#include "geometry/Point.h"


class DTriangulation {
public:
     DTriangulation();

     virtual ~DTriangulation();

     /**
      * Triangulate the specified domain
      * \see SetOuterPolygone
      * \see AddHole
      */
     void Triangulate();

     /**
      * @return the triangles resulting from the triangulation
      */
     std::vector<p2t::Triangle*>  GetTriangles() {
          return _cdt->GetTriangles();
     }

     /**
      * Set the boundaries of the domain
      * @param outerConstraints
      */
     void SetOuterPolygone(const std::vector<Point>&  outerConstraints);

     /**
      * Add a new hole
      * A domain can contains holes.
      * They should fully be inside the domain.
      */
     void AddHole(const std::vector<Point>&  hole);

     //templates for freeing and clearing a vector of pointers
     template <class C> void FreeClear( C & cntr ) {
          for ( typename C::iterator it = cntr.begin();
                    it != cntr.end(); ++it ) {
               delete * it;
          }
          cntr.clear();
     }

private:
     std::vector< std::vector<p2t::Point*> > _holesPolylines;
     std::vector<p2t::Point*> _outerConstraintsPolyline;
     p2t::CDT* _cdt;

};
