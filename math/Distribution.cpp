/* 
 * File:   Distribution.cpp
 * Author: andrea
 * 
 * Created on 17. Februar 2011, 15:14
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

    if (x > EPS) {
        pQueue.push_back(fabs(x));
    }
    if (y > EPS) {
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
