/*Mathematics.h
  Copyright (C) <2009-2010>  <Jonas Mehlich and Mohcine chraibi>

  This file is part of OpenPedSim.

  OpenPedSim is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  OpenPedSim is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenPedSim.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

#include <complex>
#include <cmath>
using namespace std;

double sign(double x);

double hermite_interp(double x, double x1, double x2, double y1, double y2,
        double dy1, double dy2);

complex<double> c_cbrt(complex<double> x);


#endif /*MATHEMATICS_H_*/
