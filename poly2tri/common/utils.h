/**
 * \file        utils.h
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


#ifndef UTILS_H
#define UTILS_H

// Otherwise #defines like M_PI are undeclared under Visual Studio
#define _USE_MATH_DEFINES
#define M_PI            3.14159265358979323846
#include <exception>
#include <math.h>

namespace p2t {

const double PI_3div4 = 3 * M_PI / 4;
const double PI_div2 = 1.57079632679489661923;
const double EPSILON = 1e-12;

enum Orientation { CW, CCW, COLLINEAR };

/**
 * Forumla to calculate signed area<br>
 * Positive if CCW<br>
 * Negative if CW<br>
 * 0 if collinear<br>
 * <pre>
 * A[P1,P2,P3]  =  (x1*y2 - y1*x2) + (x2*y3 - y2*x3) + (x3*y1 - y3*x1)
 *              =  (x1-x3)*(y2-y3) - (y1-y3)*(x2-x3)
 * </pre>
 */
Orientation Orient2d(Point& pa, Point& pb, Point& pc)
{
     double detleft = (pa.x - pc.x) * (pb.y - pc.y);
     double detright = (pa.y - pc.y) * (pb.x - pc.x);
     double val = detleft - detright;
     if (val > -EPSILON && val < EPSILON) {
          return COLLINEAR;
     } else if (val > 0) {
          return CCW;
     }
     return CW;
}

/*
  bool InScanArea(Point& pa, Point& pb, Point& pc, Point& pd)
  {
  double pdx = pd.x;
  double pdy = pd.y;
  double adx = pa.x - pdx;
  double ady = pa.y - pdy;
  double bdx = pb.x - pdx;
  double bdy = pb.y - pdy;

  double adxbdy = adx * bdy;
  double bdxady = bdx * ady;
  double oabd = adxbdy - bdxady;

  if (oabd <= EPSILON) {
  return false;
  }

  double cdx = pc.x - pdx;
  double cdy = pc.y - pdy;

  double cdxady = cdx * ady;
  double adxcdy = adx * cdy;
  double ocad = cdxady - adxcdy;

  if (ocad <= EPSILON) {
  return false;
  }

  return true;
  }

*/

bool InScanArea(Point& pa, Point& pb, Point& pc, Point& pd)
{
     double oadb = (pa.x - pb.x)*(pd.y - pb.y) - (pd.x - pb.x)*(pa.y - pb.y);
     if (oadb >= -EPSILON) {
          return false;
     }

     double oadc = (pa.x - pc.x)*(pd.y - pc.y) - (pd.x - pc.x)*(pa.y - pc.y);
     if (oadc <= EPSILON) {
          return false;
     }
     return true;
}

}

#endif

