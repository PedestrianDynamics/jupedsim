/**
 * \file        Mathematics.cpp
 * \date        Dec 13, 2010
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
 * This class is used to define mathematical parameters, constants and functions.
 *
 **/
#include "Mathematics.h"

#include <cassert>

// ok that is not perfect. For a profound discussion see http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
bool almostEqual(double a, double b, double eps)
{
    // std::cout<< "a=" << a << "  b=" << b<< "diff= "<<std::fabs(a-b)<<std::endl;
    return fabs(a - b) < eps; //std::numeric_limits<double>::epsilon();
}

/*
 * Determines the sign of the number x.
 * Returns -1 if x is negativ else 1.
 */
double sign(double x)
{
    return (x < 0) ? -1.0 : 1.0;
}

/// http://en.wikipedia.org/wiki/Sigmoid_function
/// b is negative
/// a is positive

double sigmoid(double a, double b, double x)
{
    return a / (1 + exp(b * x));
}
// thanks to Sean Curtis
double hermite_interp(double t, double x1, double x2, double y1, double y2, double dy1, double dy2)
{
    assert(t >= x1 && t <= x2 && "Can only interpolate values inside the range");
    assert(x2 > x1 && "Intervals must be defined as x1 < x2");

    double scale = x2 - x1;
    t            = (t - x1) / scale;
    double t2    = t * t;
    double t3    = t2 * t;
    double h1    = 2 * t3 - 3 * t2 + 1;
    double h2    = -2 * t3 + 3 * t2;
    double h3    = t3 - 2 * t2 + t;
    double h4    = t3 - t2;
    double left  = y1 * h1 + dy1 * h3 * scale;
    double right = y2 * h2 + dy2 * h4 * scale;
    return left + right;
}


/* Principal cubic root of a complex number */
std::complex<double> c_cbrt(std::complex<double> x)
{
    double a, b, r, phi, rn;
    std::complex<double> I(0, 1);
    double s;
    a = real(x);
    b = imag(x);

    r = sqrt(a * a + b * b);

    phi = atan2(b, a);
    phi /= 3.0;
    rn = cbrt(r);
    s  = sin(phi);
    return rn * cos(phi) + I * rn * s;
}
