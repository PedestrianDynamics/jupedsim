/* 
 * File:   Point.h
 * Author: andrea
 *
 * Created on 30. September 2010, 09:21
 */

#ifndef _POINT_H
#define	_POINT_H

#include <cmath>
#include <cstdio>
#include "../general/Macros.h"

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

    Point CoordTransToEllipse(const Point& center, double cphi, double sphi) const; // Verschiebung und Drehung
    Point CoordTransToCart(const Point& center, double cphi, double sphi) const; // Verschiebung und Drehung
    Point Rotate(double ctheta, double stheta) const; // rotiert Vektor um den Winkel theta


    // überladene Operatoren
    const Point operator+(const Point& p) const; // Addiert zwei Vektoren
    const Point operator-(const Point& p) const; // Subtrahiert zwei Vektoren
    bool operator==(const Point& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise
    bool operator!=(const Point& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise
};


// Operatoren mit double
const Point operator*(const Point& p, const double f);
const Point operator/(const Point& p, const double f);

#endif	/* _POINT_H */

