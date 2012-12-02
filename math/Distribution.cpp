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

Distribution::Distribution() {
    pMean = 0.;
    pSigma = 1.;
}

Distribution::Distribution(double m, double s) {
    pMean = m;
    pSigma = s;
}

Distribution::Distribution(const Distribution& orig) {
    pMean = orig.GetMean();
    pSigma = orig.GetSigma();
}

Distribution::~Distribution() {
}

// Getter-Funktionen

double Distribution::GetMean() const {
    return pMean;
}

double Distribution::GetSigma() const {
    return pSigma;
}

/*************************************************************
 Gauss
 ************************************************************/

const vector<double>& Gauss::GetQueue() const {
    return pQueue;
}

void Gauss::GetPair() {
    double x1 = (double) rand() / (double) RAND_MAX;
    double x2 = (double) rand() / (double) RAND_MAX;
    double x = GetMean() + sqrt(-2. * GetSigma() * log(x1)) * cos(2. * M_PI * x2);
    double y = GetMean() + sqrt(-2. * GetSigma() * log(x1)) * sin(2. * M_PI * x2);

    if (x > J_EPS) {
        pQueue.push_back(fabs(x));
    }
    if (y > J_EPS) {
        pQueue.push_back(fabs(y));
    }
}

Gauss::Gauss() : Distribution() {
    pQueue = vector<double>();
}

Gauss::Gauss(double m, double s) : Distribution(m, s) {
    pQueue = vector<double>();
}

Gauss::Gauss(const Gauss& orig) : Distribution(orig) {
    pQueue = orig.GetQueue();
}

Gauss::~Gauss() {
    pQueue.clear();
}

double Gauss::GetRand() {
    if (!pQueue.size()) {
        GetPair();
    }

    double r = pQueue.back(); // nur ein Wert wird zurück gegeben
    pQueue.pop_back(); // andere Wert kommt zurück in die Queue
    return r;

}

/*************************************************************
 Equal
 ************************************************************/

Equal::Equal() : Distribution() {
}

Equal::Equal(double m, double s) : Distribution(m, s) {
}

Equal::Equal(const Gauss& orig) : Distribution(orig) {
}

Equal::~Equal() {
}
// Gleichverteilte ZUfallszahl auf [mu-sigma, mu+sigma]
double Equal::GetRand() {
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
