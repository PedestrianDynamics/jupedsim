/* 
 * File:   Point.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 09:21
 */

#include "Point.h"

/************************************************************
 private
 ************************************************************/

// rotiert Vektor um den Winkel theta

Point Point::Rotate(double ctheta, double stheta) const {
    return Point(pX * ctheta - pY*stheta, pX * stheta + pY * ctheta);
}

/************************************************************
  Konstruktoren
 ************************************************************/
Point::Point() {
    pX = 0.0;
    pY = 0.0;
}

Point::Point(double x, double y) {
    pX = x;
    pY = y;
}

Point::Point(const Point& orig) {
    pX = orig.GetX();
    pY = orig.GetY();
}

//Point::~Point() {
//}

/*************************************************************
 Setter-Funktionen
 ************************************************************/

void Point::SetX(double x) {
    pX = x;
}

void Point::SetY(double y) {
    pY = y;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

double Point::GetX() const {
    return pX;
}

double Point::GetY() const {
    return pY;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/

// Norm des Vektors

double Point::Norm() const {
    return sqrt(pX * pX + pY * pY);
}

// Norm des Vektors zum quadrat
double Point::NormSquare() const {
	return (pX * pX + pY * pY);
}

// gibt den normierten Vector zurueck

Point Point::Normalized() const {
	double norm=NormSquare();

    if (norm != 0)
        return (Point(pX, pY) / sqrt(norm));
    else return Point(0.0, 0.0);
}

// Skalarprodukt zweier Vektoren

double Point::ScalarP(const Point& v) const {
    return pX * v.GetX() + pY * v.GetY();
}

/* Transformiert die "normalen" Koordinaten in Koordinaten der Ellipse
 * dazu verschieben des Koordinaten Ursprungs in Center und anschliessend drehen um phi
 * alle Punkte müssen in "normalen" Koordinaten gegeben sein
 * center: Center der Ellipse in deren System transformiert werden soll
 * phi: Winkel der Ellipse in deren System transformiert werden soll
 * */

Point Point::CoordTransToEllipse(const Point& center, double cphi, double sphi) const {
    Point p = Point(pX, pY);
    return (p - center).Rotate(cphi, -sphi);
}

/* Transformiert den Punkt aus dem Koordinatensystem der Ellipse
 * zurueck ins urspruengliche "normale" Koordinatensystem
 * center: Center der Ellipse aus deren System transformiert werden soll
 * phi: Winkel der Ellipse aus deren System transformiert werden soll
 * */
Point Point::CoordTransToCart(const Point& center, double cphi, double sphi) const {
    Point p = Point(pX, pY);
    return (p.Rotate(cphi, sphi) + center);
}

/*************************************************************
 überladene Operatoren
 ************************************************************/

// Addiert zwei Vektoren

const Point Point::operator+(const Point& p) const {
    return Point(pX + p.GetX(), pY + p.GetY());
}

// Subtrahiert zwei Vektoren

const Point Point::operator-(const Point& p) const {
    return Point(pX - p.GetX(), pY - p.GetY());
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool Point::operator==(const Point& p) const {
    return (fabs(pX - p.GetX()) < EPS && fabs(pY - p.GetY()) < EPS);
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool Point::operator!=(const Point& p) const {
    return (fabs(pX - p.GetX()) > EPS || fabs(pY - p.GetY()) > EPS);
}


// Multipliziert einen Vektor mit einem Skalar

const Point operator*(const Point& p, double f) {
    return Point(p.GetX() * f, p.GetY() * f);
}

// Dividiert einen Vektor durch einen Skalar

const Point operator/(const Point& p, double f) {
    return Point(p.GetX() / f, p.GetY() / f);
}
