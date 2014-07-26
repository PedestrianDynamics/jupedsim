/**
 * \file        testClassLine.cpp
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
 
 
#include <cstdlib>
#include <stdio.h>
#include "../geometry/Line.h"
#include<math.h>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

const double eps = 0.00001;
const double pi= atan(1)*4;


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
     fprintf (f, "\t+++++ Leave testIntersectionWith() +++++\n\n");
     return (res==ntests)?1:0;
}

int testGetAngle(FILE * f)
{
     fprintf (f, "\t+++++ Enter testGetAngle() +++++\n");
     int ntests=9,
         res=0;
     double angle;
     Line L1 = Line( Point(1, 2), Point(3, 4));
     Line L2 = Line( Point(0, 3), Point(3, 3));
     angle = L1.GetAngle(L2);
     if ( fabs(pi/4 + angle)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f | \
L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L1 = Line(  Point(3, 4), Point(1, 2));
     angle = L1.GetAngle(L2);
     if (fabs(pi/4 + angle)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////

     L1 = Line(  Point(1, 4),  Point(3, 2));
     angle = L1.GetAngle(L2);
     if (fabs(angle - pi/4)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L1 = Line(  Point(3, 2), Point(1, 4));
     angle = L1.GetAngle(L2);
     if (fabs(angle-pi/4)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                         \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),       \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),       \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),       \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());


////////////////////////////////////////////////
     L2 = Line(  Point(3, 3), Point(0, 3));
     angle = L1.GetAngle(L2);
     if (fabs(angle-pi/4)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                            \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());
////////////////////////////////////////////////
     L2 = Line(  Point(1, 1), Point(3, 3));
     L1 = Line(  Point(3, 2), Point(0, 2));
     angle = L1.GetAngle(L2);
     if (fabs(angle-pi/4)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                            \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

////////////////////////////////////////////////
     L2 = Line(  Point(3, 3), Point(1, 1) );
     L1 = Line(  Point(3, 2), Point(0, 2));
     angle = L1.GetAngle(L2);
     if (fabs(angle-pi/4)< 0.001)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                            \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());


////////////////////////////////////////////////
     L2 = Line(  Point(6, 5), Point(10, 7) );
     L1 = Line(  Point(6,5), Point(7.4, 9.28));
     angle = L1.GetAngle(L2);
     if (fabs(angle+pi/4)< 1)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                            \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());


////////////////////////////////////////////////
     L1 = Line(  Point(8.0467, 6.9756), Point(10.08, 2.9578) );
     L2 = Line(  Point(8.0467, 6.9756), Point(6.63, 2.733));
     angle = L1.GetAngle(L2);
     if (fabs(angle+pi/4)< 1)
          res++;
     fprintf (f, "%2d. res=%2d, A=%.2f |  \
        L1_P1(%.2f, %.2f), L1_P2(%.2f, %.2f),   \
        L2_P1(%.2f, %.2f) L2_P2(%.2f, %.2f)\n", \
              ntests, res, angle*180/pi,                            \
              L1.GetPoint1().GetX(),L1.GetPoint1().GetY(),          \
              L1.GetPoint2().GetX(),L1.GetPoint2().GetY(),          \
              L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),          \
              L2.GetPoint2().GetX(),L2.GetPoint2().GetY());

     int i;
     double phi=0, imax = 10; //rotation angle for tests
     Line L3; //the rotation of L1
     L2 = Line(  Point(0, 0), Point( 3, 0));
     L1 = Line(  Point(0, 0), Point(5, 0) );
     Point P1, P2;

     for (i=1; i<= imax; i++) {
          L3 = L1;
          phi += pi/imax;
          P1 =  L1.GetPoint2().Rotate(cos(phi), sin(phi)) ;
          L3.SetPoint2( P1 );
          angle = L3.GetAngle(L2);
          P2 = P1.Rotate(cos(angle), sin(angle)) ;

// angle  L1_P1 L1_P2   L2_P1   L2_P2    L3_P1    L3_P2
          fprintf (stderr, "%f %f\t %.2f %.2f %.2f %.2f \t %.2f %.2f %.2f %.2f\t %.2f %.2f %.2f %.2f\n",       \
                   phi, angle,                                                \
                   L3.GetPoint1().GetX(),L3.GetPoint1().GetY(),              \
                   P1.GetX(), P1.GetY(),              \
                   L2.GetPoint1().GetX(),L2.GetPoint1().GetY(),              \
                   L2.GetPoint2().GetX(),L2.GetPoint2().GetY(),              \
                   L3.GetPoint1().GetX(),L3.GetPoint1().GetY(),              \
                   P2.GetX(), P2.GetY());
     }


////////////////////////////////////////////////
     fprintf (f, "\t+++++ Leave testGetAngle() +++++\n\n");
     return (res==ntests)?1:0;
}

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

     res += testGetAngle(f);
     ntests++;

     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}


