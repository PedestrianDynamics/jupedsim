/**
 * \file        testClassPoint.cpp
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
#include "../geometry/Point.h"
#include<math.h>
//#include "Config.h"

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

const double eps = 0.00001;
const double pi= atan(1)*4;
int testNorm(FILE * f)
{
     fprintf (f, "\t+++++ Enter testNorm() +++++\n");
     int ntests=0, res=0;
     Point P(0,0); //test point
     double pnorm, expected, px, py;
     //------------------------------------------------------------------------------
     pnorm = P.Norm();
     expected = 0;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n", ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 1;
     py = 0;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 1;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = -1;
     py = 0;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 1;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 0;
     py = 1;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 1;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 0;
     py = -1;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 1;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 1;
     py = 1;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = sqrt(2);
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 3;
     py = 0;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 3;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //------------------------------------------------------------------------------
     px = 0;
     py = -3;
     P.SetX(px);
     P.SetY(py);
     pnorm = P.Norm();
     expected = 3;
     res += ( pnorm == expected )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), norm=%.2f\n",ntests, res, P._x, P._y, pnorm);
     //-------------------------------------------------------------------------------
     fprintf (f, "\t+++++ Leave testNorm() +++++\n\n");
     return (res==ntests)?1:0;
}

int testNormalized(FILE * f)
{
     fprintf (f, "\t+++++ Enter testNormalized() +++++\n");
     int ntests=0, res=0;
     Point P(0,0), nP(0,0), eP(0,0);
     //-----------------------------------------------------------------------------
     nP = P.Normalized();
     eP._x = 0;
     eP._y = 0;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     P._x = -0.5;
     P._y = 0;
     nP = P.Normalized();
     eP._x = -1;
     eP._y = 0;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     P._x = 0;
     P._y = -0.5;
     nP = P.Normalized();
     eP._x = 0;
     eP._y = -1;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     P._x = -0.5;
     P._y = 0.5;
     nP = P.Normalized();
     eP._x = -1./sqrt(2);
     eP._y = -eP._x;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     P._x = 1000.432;
     P._y = 0;
     nP = P.Normalized();
     eP._x = 1;
     eP._y = 0;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     P._x = 0;
     P._y = -12345.789;
     nP = P.Normalized();
     eP._x = 0;
     eP._y = -1;
     res += ( nP == eP )?1:0;
     ntests++;
     fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), nP(%.2f, %.2f), eP(%.2f, %.2f)\n", ntests, res, P._x, P._y, nP._x, nP._y, eP._x, eP._y );
     //------------------------------------------------------------------------------
     fprintf (f, "\t+++++ Leave testNormalized() +++++\n\n");
     return (res==ntests)?1:0;
}

int testScalarP(FILE * f)
{
     fprintf (f, "\t+++++ Enter testScalarP() +++++\n");
     Point Points[] = {
          Point(1, 2),
          Point(0, 0),
          Point(1, 0),
          Point(cos(pi/3), sin(pi/3)),
          Point(cos(pi/2), sin(pi/2)),
          Point(1,0),
          Point(cos(-pi/3), sin(-pi/3)),
          Point(cos(-pi/6), sin(-pi/6)),
          Point(cos(5*pi/6), sin(-2*pi/6)),
     };
     double expected [] = {
          0,
          0,
          cos(pi/3),
          0.8660254037844386,
          0,
          0.5,
          0.8660254037844386,
          -0.31698729810778087
     };
     const int NPOINTS = sizeof(Points) / sizeof(*Points);
     fprintf (f, " %d test cases\n\n", NPOINTS);
     int ntests=0, res=0;
     double dot=0, edot=0;

     for(int i=0; i<NPOINTS-1; i++) {
          dot =  Points[i].ScalarP(Points[i+1]);
          edot = expected[i];
          res += ( fabs(dot-edot)<0.001 )?1:0;
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), R(%.2f, %.2f), dot=%f  edot=%f\n", ntests, res, Points[i]._x, Points[i]._y, Points[i+1]._x, Points[i+1]._y, dot, edot);
          dot =  Points[i+1].ScalarP(Points[i]);
          res += ( fabs(dot-edot)<eps )?1:0;
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), R(%.2f, %.2f), dot=%f  edot=%f\n\n", ntests, res, Points[i+1]._x, Points[i+1]._y, Points[i]._x, Points[i]._y, dot, edot);
     }
     fprintf (f, "\t+++++ Leave testScalarP() +++++\n\n");
     return (res == ntests)?1:0;
}

int testDet(FILE * f)
{
     fprintf (f, "\t+++++ Enter testDet() +++++\n");
     int ntests=0, res=0;
     double det=0, edet=0;
     Point Points[] = {
          Point(1, 2),
          Point(0, 0),
          Point(1, 0),
          Point(cos(pi/3), sin(pi/3)),
          Point(cos(pi/2), sin(pi/2)),
          Point(1,0),
          Point(cos(-pi/3), sin(-pi/3)),
          Point(cos(-pi/6), sin(-pi/6)),
          Point(cos(5*pi/6), sin(-2*pi/6)),
     };
     double expected [] = {
          0,
          0,
          0.866025403784,
          0.5,
          -1.0,
          -0.866025403784,
          0.5,
          -1.18301270189
     };
     const int NPOINTS = sizeof(Points) / sizeof(*Points);
     fprintf (f, " %d test cases\n\n", NPOINTS);
     for(int i=0; i<NPOINTS-1; i++) {
          det =  Points[i].Det(Points[i+1]);
          edet = expected[i];
          res += ( fabs(det-edet)<0.001 )?1:0;
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), R(%.2f, %.2f), det=%f  edet=%f\n", ntests, res, Points[i]._x, Points[i]._y, Points[i+1]._x, Points[i+1]._y, det, edet);
          det =  Points[i+1].Det(Points[i]);
          res += ( fabs(det+edet)<eps )?1:0; //edet == -edet
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), R(%.2f, %.2f), det=%f  edet=%f\n\n", ntests, res, Points[i+1]._x, Points[i+1]._y, Points[i]._x, Points[i]._y, det, edet);
     }

     fprintf (f, "\t+++++ Leave testDet() +++++\n\n");
     return (res==ntests)?1:0;
}

int testCoordTransToEllipse(FILE * f)
{
     fprintf (f, "\t+++++ Enter testCoordTransToEllipse() +++++\n");
     int ntests=0, res=0;
     Point Points[] = {
          Point(1, 2),
          Point(0, 0),
          Point(1, 0),
          Point(cos(pi/3), sin(pi/3)),
          Point(cos(pi/2), sin(pi/2)),
          Point(1,0),
          Point(cos(-pi/3), sin(-pi/3)),
          Point(cos(-pi/6), sin(-pi/6)),
          Point(cos(5*pi/6), sin(-2*pi/6)),
     };
     const int NPOINTS = sizeof(Points) / sizeof(*Points);
     double cosphi=1, sinphi=0;
     Point transPoint(0,0), expectedPoint(0,0);
     Point Center(0,0);
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToEllipse(Center, cosphi, sinphi);
          res += ( transPoint == Points[i] )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y);
     }
//--------------------------------------------------------------------------------------
     cosphi = 0.5; //  pi/3
     sinphi = 0.8660254037844386; //pi/3
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToEllipse(Center, cosphi, sinphi);
          expectedPoint = Points[i].Rotate(cosphi, -sinphi);
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);
     }
//--------------------------------------------------------------------------------------
     cosphi = 0.5; //  -pi/3
     sinphi = -0.8660254037844386; //  -pi/3
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToEllipse(Center, cosphi, sinphi);
          expectedPoint = Points[i].Rotate(cosphi, -sinphi);
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);
     }
//--------------------------------------------------------------------------------------
     Center.SetX(1);
     Center.SetY(1);
     cosphi = 0.5; //  pi/3
     sinphi = 0.8660254037844386; //pi/3
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToEllipse(Center, cosphi, sinphi);
          expectedPoint = (Points[i]-Center).Rotate(cosphi, -sinphi);
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);
     }
     fprintf (f, "\t+++++ Leave testCoordTransToEllipse() +++++\n\n");
     return (res==ntests)?1:0;
}


int testCoordTransToCart(FILE * f)
{
     fprintf (f, "\t+++++ Enter testCoordTransToCart() +++++\n");
     int ntests=0, res=0;
     Point Points[] = {
          Point(1, 2),
          Point(0, 0),
          Point(1, 0),
          Point(cos(pi/3), sin(pi/3)),
          Point(cos(pi/2), sin(pi/2)),
          Point(1,0),
          Point(cos(-pi/3), sin(-pi/3)),
          Point(cos(-pi/6), sin(-pi/6)),
          Point(cos(5*pi/6), sin(-2*pi/6)),
     };

     const int NPOINTS = sizeof(Points) / sizeof(*Points);
     double cosphi=1, sinphi=0;
     Point transPoint(0,0), expectedPoint(0,0);
     Point Center(0,0);
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToCart(Center, cosphi, sinphi);
          res += ( transPoint == Points[i] )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y);
     }
//--------------------------------------------------------------------------------------
     cosphi = 0.5; //  pi/3
     sinphi = 0.8660254037844386; //pi/3
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToCart(Center, cosphi, sinphi);
          expectedPoint = Points[i].Rotate(cosphi, sinphi);
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;
          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);
     }
//--------------------------------------------------------------------------------------
     cosphi = 0.5; //  -pi/3
     sinphi = -0.8660254037844386; //  -pi/3
     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);



     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToCart(Center, cosphi, sinphi);
          expectedPoint = Points[i].Rotate(cosphi, sinphi);
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;

          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);

     }
//--------------------------------------------------------------------------------------

     Center.SetX(1);
     Center.SetY(1);
     cosphi = 0.5; //  pi/3

     sinphi = 0.8660254037844386; //pi/3

     fprintf (f, "\t\tcosphi=%.2f, sinphi=%.2f, Center(%.2f, %.2f)\n", cosphi, sinphi, Center._x, Center._y);
     for(int i=0; i<NPOINTS-1; i++) {
          transPoint = Points[i].CoordTransToCart(Center, cosphi, sinphi);

          expectedPoint = Points[i].Rotate(cosphi, sinphi)+Center;
          res += ( transPoint == expectedPoint )?1:0; //phi==0!
          ntests++;

          fprintf (f, "%2d. res=%2d, P(%.2f, %.2f), T(%.2f, %.2f), E(%.2f, %.2f)\n", ntests, res, Points[i]._x, Points[i]._y, transPoint._x, transPoint._y, expectedPoint._x, expectedPoint._y);
     }

     fprintf (f, "\t+++++ Leave testCoordTransToCart() +++++\n\n");
     return (res==ntests)?1:0;
}

int testRotate(FILE * f)
{

     fprintf (f, "\t+++++ Enter testRotate() +++++\n");
     int ntests=0, res=0;
     Point Points[] = {
          Point(1, 2),
          Point(0, 0),

          Point(1, 0),
          Point(cos(pi/3), sin(pi/3)),
          Point(cos(pi/2), sin(pi/2)),
          Point(1,0),
          Point(cos(-pi/3), sin(-pi/3)),
          Point(cos(-pi/6), sin(-pi/6)),
          Point(cos(5*pi/6), sin(-2*pi/6)),
     };
     double expected [] = {
          0,
          0,
          cos(pi/3),
          0.8660254037844386,
          0,
          0.5,
          0.8660254037844386,
          -0.31698729810778087
     };
     const int NPOINTS = sizeof(Points) / sizeof(*Points);
     fprintf (f, " %d test cases\n\n", NPOINTS);

     fprintf (f, "\t+++++ Leave testRotate() +++++\n\n");
     return (res==ntests)?1:0;
}


int main()
{
     FILE * f;
     char fname[FILENAME_MAX] = "log_testClassPoint.txt";
     char cCurrentPath[FILENAME_MAX];
     f = fopen(fname, "w");
     if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
          return EXIT_FAILURE;
     }
     cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

     fprintf (f, "The current working directory is %s\n\n", cCurrentPath);

     int res = 0, ntests = 0;

     res += testNorm(f);
     ntests++;
     res += testNormalized(f);
     ntests++;
     res += testScalarP(f);
     ntests++;
     res += testDet(f);
     ntests++;
     res += testCoordTransToEllipse(f);
     ntests++;
     res += testCoordTransToCart(f);
     ntests++;
     res += testRotate(f);
     ntests++;

     fclose(f);
     return (res==ntests)?EXIT_SUCCESS:EXIT_FAILURE;
}
