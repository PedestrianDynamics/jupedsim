/**
 * File:   Mathematics.h
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

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

#include <complex>
#include <cmath>

double sign(double x);

double hermite_interp(double x, double x1, double x2, double y1, double y2,
        double dy1, double dy2);

std::complex<double> c_cbrt(std::complex<double> x);


#endif /*MATHEMATICS_H_*/
