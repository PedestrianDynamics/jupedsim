/**
 * \file        testEffectiveDistanceToEllipse.cpp
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
#include <time.h>

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

OutputHandler* Log;

int testEffectiveDistanceToEllipse()
{
     clock_t t1, t2;
     char cCurrentPath[FILENAME_MAX];
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     FILE * f;
     char fname[FILENAME_MAX] = "log_testEffectiveDistanceToEllipse.txt";
     f = fopen(fname, "w");
     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);
     double dist, effdist;
     int  res=0;
     double a=2.0,  // semi-axis
            b=1.5;     // orthogonal semi-axis
     int ntests=0;
     JEllipse E1, E2;
     double x2, y2; //E2 center
     t1 = clock();

     E1.SetCenter( Point(0,0) );
     E1.SetV0(1);
     E1.SetV( Point(0,0) );
     E1.SetAmin(a);
     E1.SetBmin(b);
//    case 1
     x2 = 10;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);

     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected 10)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1, dist, effdist, x2-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++;
//     case 2
     x2 = -10;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected 10)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, effdist, fabs(x2)-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++;

//     case 3 contact
     x2 = 2*a;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(x2), effdist, fabs(x2)-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++;

//     case 4 overlap
     x2 = 2*a-1.;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(x2), effdist, fabs(x2)-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++;

//     case 6 y-axis
     x2 = 0.;
     y2 = 5;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 7 y-axis
     x2 = 0.;
     y2 = -5;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 8 y-axis
     x2 = 0.;
     y2 = 2*b;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmax(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 9 y-axis
     x2 = 0.;
     y2 = -2*b;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 10 y-axis overlap
     x2 = 0.;
     y2 = -2*b+b;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 11 y-axis overlap
     x2 = 0.;
     y2 = 2*b - b;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(y2)-2*b*0.5))  && (dist == fabs(y2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(y2), effdist, fabs(y2)-2*b,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;

//     case 5 total overlap
     x2 = 0.;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%.2f, %.2f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(x2), effdist, fabs(x2)-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;


//     case 12 ~total overlap
     x2 = 0.001;
     y2 = 0;
     E2.SetCenter( Point(x2, y2) );
     E2.SetV0(1);
     E2.SetV( Point(0,0) );
     E2.SetAmin(a);
     E2.SetBmin(b);
     effdist = E1.EffectiveDistanceToEllipse(E2, &dist);
     res += ((effdist==(fabs(x2)-2*a))  && (dist == fabs(x2)) )?1:0;
     fprintf (f, "%d. dist=%.2f (expected %.2f)\t effdist=%.2f (expected %.2f) \t E1(%.2f, %.2f)\t E2(%f, %f)\t a=%.2f\t b=%.2f\t res=%d\n",ntests+1,dist, fabs(x2), effdist, fabs(x2)-2*a,E1.GetCenter().GetX(), E1.GetCenter().GetY(), E2.GetCenter().GetX(), E2.GetCenter().GetY(), a, b, res);
     ntests++ ;


     //calculate run time
     ////////////////////////////////////////////////////////////////
     // for (int i =0; i<1000; i++)
     //     for (int j =0; j<1000;j++)
     //         E1.EffectiveDistanceToEllipse(E2, &dist);
     ///////////////////////////////////////////////////////////////
     t2 = clock();
     double elapsed_secs = double(t2 - t1) / CLOCKS_PER_SEC;
     fprintf (f, "Time:  %d clicks (%f seconds).\n",(int)(t2-t1) , elapsed_secs);

     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main()
{
     return testEffectiveDistanceToEllipse();
}
