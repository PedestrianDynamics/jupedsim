/* 
 * File:   Distribution.h
 * Author: andrea
 *
 * Created on 17. Februar 2011, 15:14
 */

#ifndef _DISTRIBUTION_H
#define	_DISTRIBUTION_H


#include <cstdlib>
#include <cmath>
#include <vector>
using namespace std;

#include "../general/Macros.h"

/* Virtuelle Klasse zur Berechnung von Zufallszahlen
 * */

class Distribution {
private:
    double pMean;
    double pSigma;
public:
    Distribution();
    Distribution(double m, double s);
    Distribution(const Distribution& orig);
    virtual ~Distribution();

    // Getter-Funktionen
    double GetMean() const;
    double GetSigma() const;

    // virtuelle Funktionen
    virtual double GetRand() =0; // Gibt EINE Zufallszahl zurück
};

class Gauss : public Distribution {
private:
    vector<double> pQueue;
    // private Getter Funktionen (werden nur innerhalb der Klasse benötigt)
    const vector<double>& GetQueue() const;
    void GetPair();
public:
    // Konstruktoren
    Gauss();
    Gauss(double m, double s);
    Gauss(const Gauss& orig);
    virtual ~Gauss();
    // Getter-Funktionen
    double GetRand(); // Gibt EINE normal verteilte Zufallszahl zurück
};

class Equal : public Distribution {
private:

public:
    // Konstruktoren
    Equal();
    Equal(double m, double s);
    Equal(const Gauss& orig);
    virtual ~Equal();
    // Getter-Funktionen
    double GetRand(); // Gibt EINE normal verteilte Zufallszahl zurück
};

#endif	/* _DISTRIBUTION_H */

