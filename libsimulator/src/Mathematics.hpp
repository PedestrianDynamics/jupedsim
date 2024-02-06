// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <cmath>
#include <complex>

bool almostEqual(double a, double b, double eps);

double sign(double x);

double sigmoid(double a, double b, double x);
double hermite_interp(double x, double x1, double x2, double y1, double y2, double dy1, double dy2);

std::complex<double> c_cbrt(std::complex<double> x);

Point mollify_e0(
    const Point& target,
    const Point& pos,
    double deltaT,
    int orientationDelay,
    const Point& e0);
