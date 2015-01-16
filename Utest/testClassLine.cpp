/**
 * \file        testClassLine.cpp
 * \date        Jul 4, 2014
 * \version     v0.6
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
 
#include <cstdlib>
#include <stdio.h>
#include "../geometry/Line.h"
#include<math.h>

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <limits>

const double pi= atan(1)*4;

//not perfect see http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// but we do not a precision of e-16
bool  almostEqual (double a, double b)
{
     // std::cout<< "a=" << a << "  b=" << b<< "diff= "<<std::fabs(a-b)<<std::endl;
     return fabs(a - b) <  0.01;//std::numeric_limits<double>::epsilon();
}

int testOperatorEqual(FILE * f)
{
     fprintf (f, "\t+++++ Enter testOperatorEqual() +++++\n");
     int ntests=10,
         res=0;

     bool bres;

     Line L1 = Line( Point(1, 1), Point(2, 3));
     Line L2 = Line( Point(1, 0), Point(2, 3));//no
     Line L3 = Line( Point(1, 1), Point(2, 2));//no
     Line L4 = Line( Point(1, 1), Point(2, 3));//yes
     Line L5 = Line( Point(2, 3), Point(1, 1));//yes
     Line L6 = Line( Point(2, 3), Point(0, 5));//no
     Line L7 = Line( Point(6, 0), Point(2, 3));//no
     Line L8 = Line( Point(1, 1), Point(3, 3));//no
     Line L9 = Line( Point(2, -1), Point(1, 1));//no
     Line L10 = Line( Point(1, 1), Point(2.1, 3));//no
     Line L11 = Line( Point(1, 1), Point(2.01, 3));//no
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L3;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L4;
     bres = (L1==L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L5;
     bres = (L1==L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L6;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L7;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L8;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L9;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L10;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L11;
     bres = (L1==L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////

////////////////////////////////////////////////
     fprintf (f, "\t+++++ Leave testOperatorEqual() +++++\n\n");
     return (res==ntests)?1:0;
} 


int testIntersectionWith(FILE * f)
{
     fprintf (f, "\t+++++ Enter testIntersectionWith() +++++\n");
     int ntests=8,
         res=0;

     bool bres;

     Line L1 = Line( Point(1, 1), Point(2, 3));
     Line L2 = Line( Point(1, 0), Point(0, 1));//no
     Line L3 = Line( Point(2, 0), Point(0, 2));//touche
     Line L4 = Line( Point(3, 0), Point(0, 3));//inter
     Line L5 = Line( Point(4, 0), Point(0, 4));//inter
     Line L6 = Line( Point(5, 0), Point(0, 5));//touche
     Line L7 = Line( Point(6, 0), Point(0, 6));//no
     Line L8 = Line( Point(2, 1), Point(3, 3));//parallel
     Line L9 = Line( Point(2, -1), Point(3, -2));//parallel
     bres = L1.IntersectionWith(L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L3;
     bres = L1.IntersectionWith(L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L4;
     bres = L1.IntersectionWith(L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L5;
     bres = L1.IntersectionWith(L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L6;
     bres = L1.IntersectionWith(L2);
     if (bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2=L7;
     bres = L1.IntersectionWith(L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

////////////////////////////////////////////////
     L2=L8;
     bres = L1.IntersectionWith(L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L1=L2;
     L2=L9;
     bres = L1.IntersectionWith(L2);
     if (!bres)
          res++;
     fprintf (f, "%2d. res=%2d, L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f), L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", ntests, res, L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),L1.GetPoint2().GetX(),L1.GetPoint2().GetY(), L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

////////////////////////////////////////////////

////////////////////////////////////////////////
     fprintf (f, "\t+++++ Leave testIntersectionWith() +++++\n\n");
     return (res==ntests)?1:0;
}

int testGetDeviationAngle(FILE * f)
{
     fprintf (f, "\t+++++ Enter testGetDeviationAngle() +++++\n");
     int ntests=4,
         res=0;
     double angle;
     Line L1 = Line( Point(0, 0), Point(-6, 0));
     Line L2 = Line( Point(-6, -5), Point(-6, -5));
     angle = L1.GetDeviationAngle(L2)*180/pi;
     if ( almostEqual(fabs(angle),39.81))
          res++;

     fprintf (f, "%2d. res=%2d, angle=%.2f (should be 39.81°) | \
L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
//////////////////////////////////////////////// -- 2
     L1 = Line( Point(0, 0), Point(0, 4));
     L2 = Line( Point(-9, 4), Point(1, 4));
     angle = L1.GetDeviationAngle(L2)*180/pi;
     if ( almostEqual(angle, -14.04))
          res++;
     fprintf (f, "%2d. res=%2d, angle=%.2f (should be -14.4°) | \
L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
//////////////////////////////////////////////// -- 3
     L1 = Line( Point(0, 0), Point(-3, -3));
     L2 = Line( Point(-11, -3), Point(2, -3));
     angle = L1.GetDeviationAngle(L2)*180/pi;
     if ( almostEqual(angle, 78.69))
          res++;

     fprintf (f, "%2d. res=%2d, angle=%.2f (should be 78.69°) | \
L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
//////////////////////////////////////////////// -- 4
     L1 = Line( Point(0, 0), Point(-2, 3));
     L2 = Line( Point(5, 10), Point(-2, 3));
     angle = L1.GetDeviationAngle(L2)*180/pi;
     if ( almostEqual(angle, 0.0))
          res++;
     fprintf (f, "%2d. res=%2d, angle=%.2f (should be 0°) | \
L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     fprintf (f, "\t+++++ Leave testGetDeviationAngle() +++++\n\n");
     return (res==ntests)?1:0;
}


// int testGetAngle(FILE * f)
// {
//      fprintf (f, "\t+++++ Enter testGetAngle() +++++\n");
//      int ntests=8,
//          res=0;
//      double angle;
//      Line L1 = Line( Point(1, 2), Point(3, 4));
//      Line L2 = Line( Point(0, 3), Point(3, 3));
//      angle = L1.GetAngle(L2);
//      if ( fabs(pi/4 + angle)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f | \
// L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
// L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                         \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
// ////////////////////////////////////////////////
//      L1 = Line(  Point(3, 4), Point(1, 2));
//      angle = L1.GetAngle(L2);
//      if (fabs(pi/4 + angle)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                         \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
// ////////////////////////////////////////////////

//      L1 = Line(  Point(1, 4),  Point(3, 2));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle - pi/4)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                         \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
// ////////////////////////////////////////////////
//      L1 = Line(  Point(3, 2), Point(1, 4));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle-pi/4)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                         \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());


// ////////////////////////////////////////////////
//      L2 = Line(  Point(3, 3), Point(0, 3));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle-pi/4)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                            \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
// ////////////////////////////////////////////////
//      L2 = Line(  Point(1, 1), Point(3, 3));
//      L1 = Line(  Point(3, 2), Point(0, 2));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle-pi/4)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                            \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

// ////////////////////////////////////////////////
//      L2 = Line(  Point(3, 3), Point(1, 1) );
//      L1 = Line(  Point(3, 2), Point(0, 2));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle-pi/4)< 0.001)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                            \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());


// ////////////////////////////////////////////////
//      L2 = Line(  Point(6, 5), Point(10, 7) );
//      L1 = Line(  Point(6,5), Point(7.4, 9.28));
//      angle = L1.GetAngle(L2);
//      if (fabs(angle+pi/4)< 1)
//           res++;
//      fprintf (f, "%2d. res=%2d, A=%.2f |  \
//         L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
//         L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
//               ntests, res, angle*180/pi,                            \
//               L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
//               L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
//               L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
//               L2.GetPoint2().GetX(),L2.GetPoint2().GetY());




////////////////////////////////////////////////
//      fprintf (f, "\t+++++ Leave testGetAngle() +++++\n\n");
//      return (res==ntests)?1:0;
// }

int main()
{
     FILE * f;
     char fname[FILENAME_MAX] = "log_testClassLine.txt";
     char cCurrentPath[FILENAME_MAX];
     f = fopen(fname, "w");
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);

     int res = 0, ntests = 0;

     res += testIntersectionWith(f);
     ntests++;

     res += testGetDeviationAngle(f);
     ntests++;

     res += testOperatorEqual(f);
     ntests++;

     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}


