/**
 * File:   Point.h
 *
 * Created on 30. September 2010, 09:21
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

#ifndef _POINT_H
#define	_POINT_H

#include <cmath>
#include <cstdio>
#include "../general/Macros.h"
#include <sstream>

class Point {
private:
    double pX;
    double pY;

public:
    // Konstruktoren
    Point();
    Point(double x, double y);
    Point(const Point& orig);
//    virtual ~Point();

    // Setter-Funktionen
    void SetX(double x); // setzt x-Koordinate
    void SetY(double y); // setzt y-Koordinate

    // Getter-Funktionen
    double GetX() const; // gibt x-Koordinate zurück
    double GetY() const; // gibt y-Koordinate zurück

    // Sonstiges
    double Norm() const; // Norm des Vektors
    double NormSquare() const; // Norm des Vektors zum quadrat
    Point Normalized() const; // normierten Vector zurück
    double ScalarP(const Point& v) const; // Skalarprodukt zweier Vektoren
    /// determinant of the square matrix formed by the vectors [ this, v]
    double Det(const Point& v) const;

    Point CoordTransToEllipse(const Point& center, double cphi, double sphi) const; // Verschiebung und Drehung
    Point CoordTransToCart(const Point& center, double cphi, double sphi) const; // Verschiebung und Drehung
    Point Rotate(double ctheta, double stheta) const; // rotiert Vektor um den Winkel theta


    // überladene Operatoren
    const Point operator+(const Point& p) const; // Addiert zwei Vektoren
    const Point operator-(const Point& p) const; // Subtrahiert zwei Vektoren
    bool operator==(const Point& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise
    bool operator!=(const Point& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise

    //debug
    std::string toString();
};


// Operatoren mit double
const Point operator*(const Point& p, const double f);
const Point operator/(const Point& p, const double f);

#endif	/* _POINT_H */

