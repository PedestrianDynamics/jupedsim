/**
 * \file        testIsOutside.cpp
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
#include "../pedestrian/Ellipse.h"

//#include "Config.h"

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

OutputHandler* Log;


//int testPointOnEllipse(int argc, char* argv[])
int testIsOutside()
{

     char cCurrentPath[FILENAME_MAX];
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     FILE * f;
     char fname[FILENAME_MAX] = "log_testIsOutside.txt";
     // strcpy(fname, cCurrentPath);
     // strcat(fname, "logs/log_testPointOnEllipse.txt");
     f = fopen(fname, "w");
     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);
     int ntests=0, res=0;
     float a=2.0, b=10.0;
     JEllipse E;
     Point P;
     //double px, py;
     E.SetCenter( Point(0,0) );
     E.SetV0(1);
     E.SetV( Point(0,0) );
     E.SetAmin(a);
     E.SetBmin(b);
     // P == E.center
     P.SetX(0);
     P.SetY(0);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%-4.2f, %-4.2f), a=%-4.2f, b=%-4.2f, P(%4.2f, %4.2f)    res=%-3d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);
     // P ~ E.center

     P.SetX(0.);
     P.SetY(-0.3);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%-4.2f, %-4.2f), a=%-4.2f, b=%-4.2f, P(%4.2f, %4.2f)    res=%-3d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

     // P ~ E.center
     P.SetX(-0.73);
     P.SetY(-0.3);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

     // P  semi-axis
     P.SetX(a);
     P.SetY(0);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);
     // P  semi-axis
     P.SetX(-a);
     P.SetY(0);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

     // P ~ semi-axis
     P.SetX(0);
     P.SetY(b*0.5);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

     // P ~ semi-axis
     P.SetX(0);
     P.SetY(-b*0.5);
     res += ( E.IsOutside(P) == false)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

     // P outside
     P.SetX(a);
     P.SetY(-b);
     res += ( E.IsOutside(P) == true)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

// P outside
     P.SetX(2*a);
     P.SetY(3.1*b);
     res += ( E.IsOutside(P) == true)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);


// P outside
     P.SetX(3*a);
     P.SetY(-3.5*b);
     res += ( E.IsOutside(P) == true)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);


// P outside
     P.SetX(-5*a);
     P.SetY(-2*b);
     res += ( E.IsOutside(P) == true)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);


// P outside
     P.SetX(-1.1*a);
     P.SetY(-1.1*b);
     res += ( E.IsOutside(P) == true)?1:0;
     ntests++;
     fprintf (f, "%3d. E(%3.2f, %3.2f), a=%3.2f, b=%3.2f, P(%3.2f, %3.2f)    res=%d\n",ntests, E.GetCenter().GetX(), E.GetCenter().GetY(), a, b, P.GetX(), P.GetY(), res);

//-----------------------------------------------------------------


     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char * argv[])
{
     //   fprintf(stdout, "%s Version %d.%d\n", argv[0], JPSCORE_MINOR_VERSION, JPSCORE_MAJOR_VERSION);
     return testIsOutside();
}
