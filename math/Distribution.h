/**
 * File:   Distribution.h
 *
 * Created on 17. Februar 2011, 15:14
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

