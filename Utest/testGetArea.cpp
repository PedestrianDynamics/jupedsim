/**
 * \file        testGetArea.cpp
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
#include "../pedestrian/Ellipse.h"

//#include "Config.h"

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

OutputHandler* Log;


//int testPointOnEllipse(int argc, char* argv[])
int testGetArea()
{
     char cCurrentPath[FILENAME_MAX];
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     FILE * f;
     char fname[FILENAME_MAX] = "log_testGetArea.txt";
     // strcpy(fname, cCurrentPath);
     // strcat(fname, "logs/log_testPointOnEllipse.txt");
     f = fopen(fname, "w");
     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);
     int ntests=0, res=0;
     double pi = 3.141592653589793;
     JEllipse E;
     double a=2.0;
     double b=4.0;
     // #1
     E.SetCenter( Point(0,0) );
     E.SetV0(1);
     E.SetV( Point(0,0) );
     E.SetAmin(a);
     E.SetBmin(b);
     res = ( fabs(E.GetArea() - a*b*0.5*pi) < 0.001 )?1:0;
     fprintf (f, "a=%.2f\t b=%.2f\t area=%.2f\t expected=%.2f\t res=%d\t\n",a, b*0.5, E.GetArea(), a*b*0.5*pi, res);
     ntests++;
     // #2
     E.SetCenter( Point(10,-4) );
     res += ( fabs(E.GetArea() - a*b*0.5*pi) < 0.001 )?1:0;
     fprintf (f, "a=%.2f\t b=%.2f\t area=%.2f\t expected=%.2f\t res=%d\t\n",a, b*0.5, E.GetArea(), a*b*0.5*pi, res);
     ntests++;
     // #3
     E.SetCenter( Point(10,-4) );
     E.SetAmin(b);
     E.SetBmin(a);
     res += ( fabs(E.GetArea() - a*b*0.5*pi) < 0.001 )?1:0;
     fprintf (f, "a=%.2f\t b=%.2f\t area=%.2f\t expected=%.2f\t res=%d\t\n",a, b*0.5, E.GetArea(), a*b*0.5*pi, res);
     ntests++;

     // #4
     E.SetCenter( Point(-12.2, 5.1) );
     a *= 0.6;
     b *= 1.3;
     E.SetAmin(a);
     E.SetBmin(b);
     res += ( fabs(E.GetArea() - a*b*0.5*pi) < 0.001 )?1:0;
     fprintf (f, "a=%.2f\t b=%.2f\t area=%.2f\t expected=%.2f\t res=%d\t\n",a, b*0.5, E.GetArea(), a*b*0.5*pi, res);
     ntests++;




     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}

int main(int argc, char * argv[])
{
     //   fprintf(stdout, "%s Version %d.%d\n", argv[0], JPSCORE_MINOR_VERSION, JPSCORE_MAJOR_VERSION);
     return testGetArea();
}
