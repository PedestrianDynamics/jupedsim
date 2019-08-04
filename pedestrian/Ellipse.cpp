/**
 * \file        Ellipse.cpp
 * \date        Sep 30, 2010
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
#include "Ellipse.h"

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif



/************************************************************
 Konstruktoren
 ************************************************************/

JEllipse::JEllipse()
{
     _vel = Point(); // velocity vector
     _center = Point(); // cartesian coordinates of the center
     _cosPhi = 1; // = cos(0)
     _sinPhi = 0; // = sin(0)
     _Xp = 0; //x Ellipse-coord of the centre (Center in (xc,yc) )
     _Amin = 0.18; // Semi-axis in direction of motion:  pAmin + V * pAv
     _Av = 0.53;
     _Bmin = 0.20; // Semi-axis in direction of shoulders: pBmax - V *[(pBmax - pBmin) / V0]
     _Bmax = 0.25;
     _do_stretch = true;
     _vel0 = 0; // desired speed
}


/*************************************************************
 Setter-Funktionen
 ************************************************************/

void JEllipse::SetV(const Point& v)
{
     _vel = v;
}

void JEllipse::SetCenter(Point pos)
{
     _center = pos;
}

void JEllipse::SetCosPhi(double c)
{
     _cosPhi = c;
}

void JEllipse::SetSinPhi(double s)
{
     _sinPhi = s;
}

void JEllipse::SetXp(double xp)
{
     _Xp = xp;
}

void JEllipse::SetAmin(double a_min)
{
     _Amin = a_min;
}

void JEllipse::SetAv(double a_v)
{
     _Av = a_v;
}

void JEllipse::SetBmin(double b_min)
{
     _Bmin = b_min;
}

void JEllipse::SetBmax(double b_max)
{
     _Bmax = b_max;
}

void JEllipse::SetV0(double v0)
{
     _vel0 = v0;
}

void JEllipse::DoStretch(bool stretch)
{
     _do_stretch = stretch;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

const Point& JEllipse::GetV() const
{
     return _vel;
}

const Point& JEllipse::GetCenter() const
{
     return _center;
}

double JEllipse::GetCosPhi() const
{
     return _cosPhi;
}

double JEllipse::GetSinPhi() const
{
     return _sinPhi;
}

double JEllipse::GetXp() const
{
     return _Xp;
}

double JEllipse::GetAmin() const
{
     return _Amin;
}

double JEllipse::GetAv() const
{
     return _Av;
}

double JEllipse::GetBmin() const
{
     return _Bmin;
}

double JEllipse::GetBmax() const
{
     if (_do_stretch)
     {
          return _Bmax;
     }
     else
          return _Bmin;
}

double JEllipse::GetV0() const
{
     return _vel0;
}
double JEllipse::GetArea() const
{
     // double x = (_Bmax - _Bmin) / _vel0;
     // double V = _vel.Norm();
     double ea = GetEA();// _Amin + V * _Av;
     double eb = GetEB(); //_Bmax - V * x;
     return ea * eb * M_PI;
}

// ellipse  semi-axis in the direction of the velocity
double JEllipse::GetEA() const
{
      return _Amin + _vel.Norm() * _Av;
}

bool JEllipse::DoesStretch() const
{
     return _do_stretch;
}

// ellipse semi-axis in the orthogonal direction of the velocity
double JEllipse::GetEB() const
{
//     double b_shoulder = _Bmin; /// width of shoulder. todo: find out empricial value
//     double v_min = 0.001;
//     double a = 0.49;
//     double b = -1.17;
//     double v = _vel.Norm();
     // double t = (v<v_min)? 0.5*b_shoulder: 0.5*(b_shoulder + a * exp(b*v));
     // printf("v=%f, b=%f\n", v, t);
     // getc(stdin);
     //return (v<v_min)? 0.5*b_shoulder: 0.5*(b_shoulder + a * exp(b*v));
     if (_do_stretch)
     {
          double x = (_vel0 <= 0.001) ? 0 : (_Bmax - _Bmin) / _vel0;
          return _Bmax - _vel.Norm() * x;
     } else {
          return _Bmin;
     }
     // double b_shoulder = _Bmin; /// width of shoulder. todo: find out empricial value
     // double v_min = 0.001;
     // double a = 0.49;
     // double b = -1.17;
     // double v = _vel.Norm();
     // double t = (v<v_min)? 0.5*b_shoulder: 0.5*(b_shoulder + a * exp(b*v));
     // printf("v=%f, b=%f\n", v, t);
     // getc(stdin);
     // return _Bmax;
     // return (v<v_min)? 0.5*b_shoulder: 0.5*(b_shoulder + a * exp(b*v));
      // todo: we dont  have the possiblity to choose between ellipses and circles.
      // for the moment we can control this only with the parameter values in the following formula

      //double x;
      //if(_vel0 > 0.001)
      //      x = (_Bmax - _Bmin) / _vel0;
      //else
      //      x = 0;

}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/

/* effektiver Abstand zwischen zwei Ellipsen (geg. durch ActionPoints),
 * Effektiver Abstand = Abstand zwischen den Randpunkten der Ellipsen
 * Algorithms:
 *   - ActionPoint von E1 (AP1) in Koordinaten von E2 transformieren
 *   - ActionPoint von E2 (AP2) in Koordinaten von E1 transformieren
 *   - R1 ist Schnittpunkt von E1 mit Geraden durch AP1 und AP2
 *     (ACHTUNG richtige Koordinaten verwenden!!!)
 *   - R2 ist Schnittpunkt von E2 mit Geraden durch AP1 und AP2
 *   - dist = ||AP1-AP2||
 *   - gesuchter effektiver Abstand = ||R1-R2||
 * Parameter:
 *   - Ellipse E2 in eigenen Koordinaten
 *   - dist: ist Rückgabewert
 * Rückgabewerte:
 *   - dist: Abstand der ActionPoints
 *   - gesuchter effektiver Abstand
 * */

// double JEllipse::EffectiveDistanceToEllipse(const JEllipse& E2, double* dist) const {
//      //  E1 ist Objekt auf dem aufgerufen wird
//      Point AP1inE1 = Point(this->GetXp(), 0); //Koords in E1
//      Point AP2inE2 = Point(E2.GetXp(), 0); //Koords in E2
//      Point AP1inE2, AP2inE1;

//      // "normale" Koordinaten
//      Point R1, R2;
//      Point AP1, AP2;
//      // Koordinaten transformieren
//      AP1 = AP1inE1.TransformToCartesianCoordinates(this->GetCenter(), this->GetCosPhi(),
//                      this->GetSinPhi());
//      AP2 = AP2inE2.TransformToCartesianCoordinates(E2.GetCenter(), E2.GetCosPhi(),
//                      E2.GetSinPhi());
//      AP1inE2 = AP1.TransformToEllipseCoordinates(E2.GetCenter(), E2.GetCosPhi(),
//                      E2.GetSinPhi());
//      AP2inE1 = AP2.TransformToEllipseCoordinates(this->GetCenter(), this->GetCosPhi(),
//                      this->GetSinPhi());

//      // Abstand zwischen den beiden Actionpoints
//      *dist = (AP1 - AP2).Norm();
//      /* fuer E1: Berechnung R1 */
//      R1 = this->PointOnEllipse(AP2inE1); // kuerzester Schnittpunkt E1 mit Gerade durch AP2 von E2
//      /* fuer E2: Berechung R2 */
//      R2 = E2.PointOnEllipse(AP1inE2); // kuerzester Schnittpunkt E2 mit Gerade durch AP1 von E1
//      // Abstand zwischen den beiden Ellipsen
//      return (AP1 - AP2).Norm() - (AP1 - R1).Norm() - (AP2 - R2).Norm(); // negative Werte sind bei Überlappung möglich
// }

/*Effective distance between two ellipses
 input: E2 (E1 -> this)

 output:
 - return: Effective distance between E1 and E2. By overlaping the distance is negative
 - (call by reference) distance between centers
 */
//Time old version:  230000 clicks (0.230000 seconds).
//Time new:  180000 clicks (0.180000 seconds)
double JEllipse::EffectiveDistanceToEllipse(const JEllipse& E2, double* dist) const
{
     //  E1 ist Objekt auf dem aufgerufen wird
     Point E1center = this->GetCenter();
     Point E2center = E2.GetCenter();
     Point R1, R2;
     Point E1inE2, //center of E1 in coordinate system of E2
           E2inE1;
     E2inE1 = E2center.TransformToEllipseCoordinates(this->GetCenter(), this->GetCosPhi(),
                                                     this->GetSinPhi());
     E1inE2 = E1center.TransformToEllipseCoordinates(E2.GetCenter(), E2.GetCosPhi(),
                                                     E2.GetSinPhi());
     // distance between centers of E1 and E2
     *dist = (E1center - E2center).Norm();
     R1 = this->PointOnEllipse(E2inE1);
     R2 = E2.PointOnEllipse(E1inE2);
     //effective distance
     return *dist - (E1center - R1).Norm() - (E2center - R2).Norm();
}

//
// input: P is a point in the ellipse world.
// output: The point on the ellipse (in cartesian coord) that lays on the same line OP
// O being the center of the ellipse
// if P approx equal to Center of ellipse return cartesian coordinats of the point (a,0)/ellipse
Point JEllipse::PointOnEllipse(const Point& P) const
{
     double x = P._x, y = P._y;
     double r = x*x + y*y;
     if ( r < J_EPS*J_EPS) {
          //return _center;
          Point CP(this->GetEA(), 0);
          return CP.TransformToCartesianCoordinates(this->GetCenter(), this->GetCosPhi(),
                                                    this->GetSinPhi());
     }
     r = sqrt(r);

     double cosTheta = x/r;
     double sinTheta = y/r;
     double a = GetEA();
     double b = GetEB();
     Point S;
     S._x = a*cosTheta;
     S._y = b*sinTheta;
     return S.TransformToCartesianCoordinates(_center, _cosPhi, _sinPhi);
}

double JEllipse::EffectiveDistanceToLine(const Line& l) const
{
     Point C = this->GetCenter();
     Point P = l.ShortestPoint(C);
     Point R = this->PointOnEllipse(P);
     return (P-R).Norm();
}

// thanks to Sean Curtis. see manuals/Ellipsen/ellipseLineSean.pdf
// double JEllipse::MinimumDistanceToLine(const Line& l) const {
//       Point AinE = l.GetPoint1().TransformToEllipseCoordinates(_center, _cosPhi, _sinPhi);
//       Point BinE = l.GetPoint2().TransformToEllipseCoordinates(_center, _cosPhi, _sinPhi);

//              // Action Point der Ellipse
//       Point APinE = Point(_Xp, 0);
//       Line linE = Line(AinE, BinE);
//       double xa = linE.GetPoint1()._x;
//       double ya = linE.GetPoint1()._y;
//       double xb = linE.GetPoint2()._x;
//       double yb = linE.GetPoint2()._y;
//       double a = GetEA();
//       double b = GetEB();
//       Line l_strich_inE;
//       // Punkt auf line mit kürzestem Abstand zum Action Point der Ellipse
//      Point PinE = linE.ShortestPoint(APinE);


//       double mindist; // Rückgabewert

//       // kürzester Punkt ist Randpunkt
//       if (PinE == AinE || PinE == BinE) {
//               mindist = 0;
//       } else {
//               double Dx, Dy      // D
//               , NormD, NormT;
//               double Nx, Ny;     // N
//               double P1x, P1y;       // P1
//               double Rx, Ry;     // R
//               double Tx, Ty;     // R
//               double d, e;
//               double dummy;
//               Dx = xa - xb;
//               Dy = ya - yb;

//               if(Dx*ya - Dy*xa < 0)
//               {
//                       Dx = -Dx;
//                       Dy = -Dy;
//               }

//               NormD = sqrt(Dx*Dx + Dy*Dy);
//               Dx /= NormD;
//               Dy /= NormD;
//               //N. The normal of the line
//               Nx = -Dy;
//               Ny = Dx;

//               Tx = -Dy/b;
//               Ty = Dx/a;
//               NormT = sqrt(Tx*Tx + Ty*Ty);
//               Tx /= NormT;
//               Ty /= NormT;

//               P1x = a*Nx;
//               P1y = b*Ny; //Eq. (2.3)

//               dummy = Nx*xa + Ny*ya; //second part of Eq. (2.1)

//               e = Nx*P1x + Ny*P1y - dummy; //Eq. (2.4)

//               //R
//               Rx = a*Tx;
//               Ry = b*Ty; // Eq. (2.13)

//               d = Nx*Rx + Ny*Ry - dummy;


// /*            if (1)
//               {
//                       printf("\n----------- dca2 --------------\n");
//                       printf("Dx = %.2f, Dy=%.2f (det=%.2f)\n", Dx, Dy, Dx*ya - Dy*xa);
//                       printf("Nx = %.2f, Ny=%.2f\n", Nx, Ny);
//                       printf("P1x = %.2f, P1y=%.2f\n", P1x, P1y);
//                       printf("Rx = %.2f, Ry=%.2f\n", Rx, Ry);
//                       printf("dummy=%f\n",dummy);
//                       printf("theta=%.2f\n",theta*180/PI);
//                       printf("e=%f, d=%f\n",e, d);
//                       printf("-------------------------\n\n");
//               }*/
//               mindist = d - e;
//       }
//       return mindist;
// }


/*
 min distance ellipses
 closest approach between two ellipses

 P1(x1,y1): Action-point in E1
 P2(x2,y2): Action-point in E2


     C1*------------*C2
      /              \
     /                \
    /                  \
   P1 *                 \
 *P2

 1. find the closest distance between C1 and C2: d
 2. translate   Ellipse2 (from right to left) with C1C2-d
 3. call edes with E1 and the translated Ellipse.

 *\  A
 *\ B
 \
-------*  C


 (xA-xB)/(xA-xC) = AB/AC
 --> xB = xA - (xA-xC)*AB/AC
 (yA-yB)/(yA-yC) = AB/AC
 --> yB = yA - (yA-yC)*AB/AC
 */
// double JEllipse::MinimumDistanceToEllipse(const JEllipse& E2) const {
//      JEllipse tmpE2 = E2; // verschobene Ellipse E2, so dass Berührung mit E1
//      Point v; // Verschiebungsvektor
//      Point tmpC2 = Point(); // verschobenes Centrum von E2
//      const Point& C1 = this->GetCenter();
//      const Point& C2 = E2.GetCenter();

//      //double d = this->Distance2d(E2); //dist closest approach between centres
//      double d = 0.0; //this->Distance2d(E2); //dist closest approach between centres
//      double distance; //between c1 and c2
//      double mind; // Rueckgabewert
//      if (d < 0 || d != d) {
//              char tmp[CLENGTH];
//              sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() d=%f\n", d);
//              Log->Write(tmp);
//              exit(0);
//      }

//      if ((C1 - C2).NormSquare() < J_EPS*J_EPS) {
//              char tmp[CLENGTH];
//              sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() m=0\n"
//                      "xc1: %f xc2: %f yc1: %f yc2: %f\n", C1._x, C2._x,
//                              C1._y, C2._y);
//              Log->Write(tmp);
//              exit(EXIT_FAILURE);
//              //return 0.0;
//      }
//      // Verschiebungsvektor bestimmen und normieren
//      v = (C2 - C1).Normalized();
//      tmpC2 = C1 + v * d;
//      tmpE2.SetCenter(tmpC2);
//      mind = this->EffectiveDistanceToEllipse(tmpE2, &distance);
//      if (mind < 0) { //Overlapping
//              mind = 0.0;
//      }
//      return mind;
// }

// check if point given in the ellipse coordinates is inside an ellipse
bool JEllipse::IsInside(const Point& p) const
{
     double a = GetEA();
     double b = GetEB();
     double x = p._x;
     double y = p._y;
     double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
     return condition <0;
     //return (x * x) / (a * a) + (y * y) / (b * b) < 1 + J_EPS_DIST;
}

// check if point given in the ellipse coordinates is outside an ellipse
bool JEllipse::IsOutside(const Point& p) const
{
     double a = GetEA();
     double b = GetEB();
     double x = p._x;
     double y = p._y;
     double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
     return condition >0;
     //return (x * x) / (a * a) + (y * y) / (b * b) > 1 - J_EPS_DIST;
}

// check if point given in the ellipse coordinates is on an ellipse
bool JEllipse::IsOn(const Point& p) const
{
     double a = GetEA();
     double b = GetEB();
     double x = p._x;
     double y = p._y;
     double condition = (x * x) / (a * a) + (y * y) / (b * b) - 1;
     return (-J_EPS_DIST < condition) && (condition < J_EPS_DIST);
     //return (-J_EPS_DIST < impliciteEllipse) && (impliciteEllipse < J_EPS_DIST);
}

/*bool Ellipse::IntersectionWithLine(const Line& line) {
        //int Mathematics::IntersectionWithLine(ELLIPSE * E, float xa, float xb, float ya, float yb){
        //      float xc = E->xc, yc = E->yc;
        //      float phi = E->phi;
        //      float xanew, yanew;
        //      float xbnew, ybnew;
        //      float delta = -1;
        //      float c,d;
        //      float a2 = E->a*E->a;
        //      float b2 = E->b*E->b;
        //      int is = 0;
        //
        //      coord_trans(xa, ya, xc, yc, phi, &xanew, &yanew);
        //      coord_trans(xb, yb, xc, yc, phi, &xbnew, &ybnew);
        //
        //
        //      if(xanew != xbnew){
        //              c = (yanew - ybnew)/(xanew - xbnew);
        //              d = yanew - c * xanew;
        //              float d2=d*d;
        //              //delta = 4*c*c*d*d/b2/b2 - 4*( d*d/b2 - 1 )*( 1.0/a2 + c*c/b2 );
        //              delta = 4*(1.0/a2 + c*c/b2 -d2/b2/a2);
        //              is = (delta < 0)?0:1;
        //      }
        //      else{
        //              is = (E->a < fabs(xanew))?0:1;
        //      }
        //      return is;
        exit(EXIT_FAILURE); // what are you looking for here?
}
*/
