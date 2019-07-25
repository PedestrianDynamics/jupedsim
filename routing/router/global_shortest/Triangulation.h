/**
 * \file        Triangulation.h
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
 *
 *
 **/


#ifndef TRIANGULATION_H_
#define TRIANGULATION_H_

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

using std::vector;
using std::endl;
using std::cout;


template <class C> void FreeClear( C & cntr )
{
     for ( typename C::iterator it = cntr.begin();
               it != cntr.end(); ++it ) {
          delete * it;
     }
     cntr.clear();
}

inline
std::vector<p2t::Triangle*> triangles ()
{
     int num_points = 0;
     double max, min;


     vector< vector<p2t::Point*> > polylines;
     vector<p2t::Point*> polyline;

     // Create a simple bounding box
     polyline.push_back(new p2t::Point(min,min));
     polyline.push_back(new p2t::Point(min,max));
     polyline.push_back(new p2t::Point(max,max));
     polyline.push_back(new p2t::Point(max,min));


     cout << "Number of constrained edges = " << polyline.size() << endl;
     polylines.push_back(polyline);


     /*
      * Perform triangulation!
      */


     /*
      * STEP 1: Create CDT and add primary polyline
      * NOTE: polyline must be a simple polygon. The polyline's points
      * constitute constrained edges. No repeat points!!!
      */
     p2t::CDT* cdt = new p2t::CDT(polyline);

     /*
      * STEP 2: Add holes or Steiner points if necessary
      */

     vector<p2t::Point*> head_hole ;
     cdt->AddHole(head_hole);
     // Add chest hole
     vector<p2t::Point*> chest_hole;
     cdt->AddHole(chest_hole);
     polylines.push_back(head_hole);
     polylines.push_back(chest_hole);

     /*
      * STEP 3: Triangulate!
      */
     cdt->Triangulate();

     /// Constrained triangles
     std::vector<p2t::Triangle*> triangles;
     /// Triangle map
     std::list<p2t::Triangle*> map;

     triangles = cdt->GetTriangles();
     map = cdt->GetMap();

     cout << "Number of points = " << num_points << endl;
     cout << "Number of triangles = " << triangles.size() << endl;



     // Free points
     for(unsigned int i = 0; i < polylines.size(); i++) {
          vector<p2t::Point*> poly = polylines[i];
          FreeClear(poly);
     }

     //    delete cdt;
     return triangles;
}

#endif /* TRIANGULATION_H_ */
