/**
 * File:   Mathematics.cpp
 *
 * Created on 13. December 2010, 15:05
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *	This class is used to define mathematical parameters, constants and functions.
 *
 *
 */

#include "Mathematics.h"




using namespace std;


/*
 * Determines the sign of the number x.
 * Returns -1 if x is negativ else 1.
 */
double sign(double x) {
    return (x < 0) ? -1.0 : 1.0;

}

/*
interpolation of f at x given [x1 x2], y1=f(x1), y2=f(x2), dy1=f'(x1), dy2=f'(x2).

Conditions: interpolation_f(x1)=y1, d_interpolation_f(x1)=dy1,   interpolation_f(x2)=y2, d_interpolation_f(x2)=dy2
Methode: Divided differences
Usage:  px = hermite_interp(x,x1,x2,y1,y2,dy1,dy2)


Theory:
  Newton interpolation
 In general
   | x1 y1    0         0            0         |
   | x2 y2 [x1 x2]y     0            0         |
 C=| x3 y3 [x2 x3]y [x1 x2 x3]y      0         |
   | x4 y4 [x3 x4]y [x2 x3 x4]y [x1 x2 x3 x4]y |

In this case (Hermite interpolation with two points)

  | x1 y1    0           0            0          |
  | x1 y1   dy1          0            0          |
C=| x2 y2 [x1 x2]y   [x1 x1 x2]y      0          |
  | x2 y2   dy2      [x1 x2 x2]y  [x1 x1 x2 x2]y |


           c[i][j-1] - c[i-1][j-1]
c[i][j] = -------------------------       : Eq (1)
           c[i][0] - c[i-j+1][0]



c3=[x1 x2 x3 x4]y
c2=[x1 x2 x3]y
c1=[x1 x2]y
c0=y1


y(x) = c0 + c1*(x-x1) + c2*(x-x1)(x-x2) + c3*(x-x1)*(x-x2)*(x-x3)

--> Hermite Interpolation:

Y(x) = y1 +  dy1*(x-x1) +  [x1 x1 x2]y*(x-x1)^2 + [x1 x1 x2 x2]y*(x-x1)^2*(x-x2)

 */
double hermite_interp(double x, double x1, double x2, double y1, double y2, double dy1, double dy2) {
    int n = 4;
    int i, j, i1;
    double c[n][n + 1];
    double px, xp1, xp2;
    //double pcoeff[n-1];
    double pcoeff[4];
    for (i = 0; i < n; i++) {
        for (j = 0; j < n + 1; j++) {
            c[i][j] = 0;
        }
    }

    // set 1st column as x1, x1, x2, x2
    c[0][0] = x1;
    c[1][0] = x1;
    c[2][0] = x2;
    c[3][0] = x2;

    // set 2nd column as y1, y1, y2, y2
    c[0][1] = y1;
    c[1][1] = y1;
    c[2][1] = y2;
    c[3][1] = y2;

    for (j = 2; j < n + 1; j++) {
        for (i = j - 1; i < n; i++) {
            if (j == 2 && i == 1) {
                c[i][j] = dy1; // 3rd column of 2nd row is y'(x1)
            } else if (j == 2 && i == 3) {
                c[i][j] = dy2; // 3rd column of 4th row is y'(x2)
            } else {
                i1 = i - j + 1;
                c[i][j] = (c[i][j - 1] - c[i - 1][j - 1]) / (c[i][0] - c[i1][0]); // Calculate using Eq.  1
            }
        }
    }
    pcoeff[0] = c[0][1]; //diag(c,1)
    pcoeff[1] = c[1][2];
    pcoeff[2] = c[2][3];
    pcoeff[3] = c[3][4];

    xp1 = x - x1;
    xp2 = x - x2; // xp1 = (x-x1), xp2 = (x-x2)
    // Calculate polynomial from coefficients using Eq. 2
    px = pcoeff[0] + pcoeff[1] * xp1 + pcoeff[2] * xp1 * xp1 + pcoeff[3] * xp1 * xp1 * xp2;
    return px;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/* Principal cubic root of a complex number */
complex<double> c_cbrt(complex<double> x) {
    double a, b, r, phi, rn;
    complex<double> I(0, 1);
    double s;
    a = real(x);
    b = imag(x);

    r = sqrt(a * a + b * b);

    phi = atan2(b, a);
    phi /= 3.0;
    rn = cbrt(r);
    s = sin(phi);
    return rn * cos(phi) + I * rn*s;
}




