// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Mathematics.hpp"

#include <cassert>

// ok that is not perfect. For a profound discussion see
// http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
bool almostEqual(double a, double b, double eps)
{
    return fabs(a - b) < eps; // std::numeric_limits<double>::epsilon();
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
    t = (t - x1) / scale;
    double t2 = t * t;
    double t3 = t2 * t;
    double h1 = 2 * t3 - 3 * t2 + 1;
    double h2 = -2 * t3 + 3 * t2;
    double h3 = t3 - 2 * t2 + t;
    double h4 = t3 - t2;
    double left = y1 * h1 + dy1 * h3 * scale;
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
    s = sin(phi);
    return rn * cos(phi) + I * rn * s;
}

Point mollify_e0(
    const Point& target,
    const Point& pos,
    double deltaT,
    int orientationDelay,
    const Point& e0)
{
    constexpr double _tau = 0.5;
    const Point delta = target - pos;
    const Point new_e0 = delta.Normalized();
    const double t = orientationDelay * deltaT;

    // Handover new target
    return e0 + (new_e0 - e0) * (1 - exp(-t / _tau));
}
