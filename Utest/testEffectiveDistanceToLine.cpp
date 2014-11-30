/**
 * \file        testEffectiveDistanceToLine.cpp
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
#include "../math/Mathematics.h"
#include "../pedestrian/Ellipse.h"


#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

OutputHandler* Log;

int testEffectiveDistanceToLine()
{
     char cCurrentPath[FILENAME_MAX];
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     FILE * f;
     char fname[FILENAME_MAX] = "log_testEffectiveDistanceToLine.txt";
     f = fopen(fname, "w");
     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);
     double dist;
     int  res=0;
     double a=2.0,  // semi-axis
            b=1.5;     // orthogonal semi-axis
     int ntests=0;
     JEllipse E;
     E.SetCenter( Point(0,0) );
     E.SetV0(1);
     E.SetV( Point(0,0) );
     E.SetAmin(a);
     E.SetBmin(b);
     // parallel y
     Point P1(2*a, 0);
     Point P2(2*a, 3);
     Line L(P1, P2);

     dist = E.EffectiveDistanceToLine(L);
     res += (dist==a)?1:0;
     ntests++;
     fprintf (f, "%2d. dist=%.2f\t Line (%.2f, %.2f)--(%.2f, %.2f) a=%.2f\t b=%.2f\t res=%d\n",ntests, dist, P1.GetX(), P1.GetY(), P2.GetX(), P2.GetY(), a, b, res);

     // parallel y
     P1.SetX(-3*a);
     P1.SetY(0);
     P2.SetX(-3*a);
     P2.SetY(3);
     L.SetPoint1( P1 );
     L.SetPoint2( P2 );

     dist = E.EffectiveDistanceToLine(L);
     res += (dist==2*a)?1:0;
     ntests++;
     fprintf (f, "%2d. dist=%.2f\t Line (%.2f, %.2f)--(%.2f, %.2f) a=%.2f\t b=%.2f\t res=%d\n",ntests, dist, P1.GetX(), P1.GetY(), P2.GetX(), P2.GetY(), a, b, res);

// parallel x
     P1.SetX(0);
     P1.SetY(2*b*0.5);
     P2.SetX(3);
     P2.SetY(2*b*0.5);
     L.SetPoint1( P1 );
     L.SetPoint2( P2 );


     dist = E.EffectiveDistanceToLine(L);
     res += (dist==b*0.5)?1:0;
     ntests++;
     fprintf (f, "%2d. dist=%.2f\t Line (%.2f, %.2f)--(%.2f, %.2f) a=%.2f\t b=%.2f\t res=%d\n",ntests, dist, P1.GetX(), P1.GetY(), P2.GetX(), P2.GetY(), a, b, res);

// parallel x
     P1.SetX(0);
     P1.SetY(-2*b*0.5);
     P2.SetX(3);
     P2.SetY(-2*b*0.5);
     L.SetPoint1( P1 );
     L.SetPoint2( P2 );


     dist = E.EffectiveDistanceToLine(L);
     res += (dist==b*0.5)?1:0;
     ntests++;
     fprintf (f, "%2d. dist=%.2f\t Line (%.2f, %.2f)--(%.2f, %.2f) a=%.2f\t b=%.2f\t res=%d\n",ntests, dist, P1.GetX(), P1.GetY(), P2.GetX(), P2.GetY(), a, b, res);


     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main()
{
     return testEffectiveDistanceToLine();
}
