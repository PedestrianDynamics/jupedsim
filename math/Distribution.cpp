/**
 * File:   Distribution.cpp
 *
 * Created on 17. February 2011, 15:14
 *
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
 *
 *
 */

#include "Distribution.h"
#include "../general/Macros.h"


#include <cstdlib>
//http://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

Distribution::Distribution()
{
     _mean = 0.;
     _sigma = 1.;
}

Distribution::Distribution(double m, double s)
{
     _mean = m;
     _sigma = s;
}

Distribution::Distribution(const Distribution& orig)
{
     _mean = orig.GetMean();
     _sigma = orig.GetSigma();
}

Distribution::~Distribution()
{
}

// Getter-Funktionen

double Distribution::GetMean() const
{
     return _mean;
}

double Distribution::GetSigma() const
{
     return _sigma;
}

/*************************************************************
 Gauss
 ************************************************************/

const vector<double>& Gauss::GetQueue() const
{
     return _queue;
}

void Gauss::GetPair()
{
     double x1 = (double) rand() / (double) RAND_MAX;
     double x2 = (double) rand() / (double) RAND_MAX;
     double x = GetMean() + sqrt(-2. * GetSigma() * log(x1)) * cos(2. * M_PI * x2);
     double y = GetMean() + sqrt(-2. * GetSigma() * log(x1)) * sin(2. * M_PI * x2);

     if (x > J_EPS) {
          _queue.push_back(fabs(x));
     }
     if (y > J_EPS) {
          _queue.push_back(fabs(y));
     }
}

Gauss::Gauss() : Distribution()
{
     _queue = vector<double>();
}

Gauss::Gauss(double m, double s) : Distribution(m, s)
{
     _queue = vector<double>();
}

Gauss::Gauss(const Gauss& orig) : Distribution(orig)
{
     _queue = orig.GetQueue();
}

Gauss::~Gauss()
{
     _queue.clear();
}

double Gauss::GetRand()
{
     if (!_queue.size()) {
          GetPair();
     }

     double r = _queue.back(); // nur ein Wert wird zurück gegeben
     _queue.pop_back(); // andere Wert kommt zurück in die Queue
     return r;

}

/*************************************************************
 Equal
 ************************************************************/

Equal::Equal() : Distribution()
{
}

Equal::Equal(double m, double s) : Distribution(m, s)
{
}

Equal::Equal(const Equal& orig) : Distribution(orig)
{
}

Equal::~Equal()
{
}

// Uniform distribution in [mu-sigma, mu+sigma]
double Equal::GetRand()
{
     double x;
     double mu = GetMean();
     double sigma = GetSigma();

     do {
          x = (double) rand() / (double) RAND_MAX; // [0,1]
          x*=2*sigma; // [0, 2*sigma]
          x+=(mu-sigma); // [mu-sigma, mu+sigma]
     } while( x< mu-sigma || mu+sigma<x);
     return x;
}
