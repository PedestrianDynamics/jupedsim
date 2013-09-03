/**
 * File:   Point.cpp
 * 
 * Created on 30. September 2010, 09:21
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

#include "Point.h"
#include "../general/Macros.h"


#include  <cmath>
#include  <sstream>





/************************************************************
  Konstruktoren
 ************************************************************/
Point::Point() {
    _x = 0.0;
    _y = 0.0;
}

Point::Point(double x, double y) {
    _x = x;
    _y = y;
}

Point::Point(const Point& orig) {
    _x = orig.GetX();
    _y = orig.GetY();
}

std::string Point::toString(){
	std::stringstream tmp;
	tmp<<"( "<<_x<<" : " <<_y<<" )";
	return tmp.str();

};


/*************************************************************
 Setter-Funktionen
 ************************************************************/

void Point::SetX(double x) {
    _x = x;
}

void Point::SetY(double y) {
    _y = y;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

double Point::GetX() const {
    return _x;
}

double Point::GetY() const {
    return _y;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/

// Norm des Vektors

double Point::Norm() const {
    return sqrt(_x * _x + _y * _y);
}

// Norm des Vektors zum quadrat
double Point::NormSquare() const {
	return (_x * _x + _y * _y);
}

// gibt den normierten Vector zurueck

Point Point::Normalized() const {
	double norm=Norm();

    if (norm != 0.0)
        return (Point(_x, _y) / (norm));
    else return Point(0.0, 0.0);
}

// Skalarprodukt zweier Vektoren
//TODO: get rid of methods call GetX, getY
double Point::ScalarP(const Point& v) const {
    return _x * v.GetX() + _y * v.GetY();
}

/// determinant of the square matrix formed by the vectors [ this, v]
double Point::Det(const Point& v) const {
	return _x * v._y - _y * v._x;
}

/* Transformiert die "normalen" Koordinaten in Koordinaten der Ellipse
 * dazu verschieben des Koordinaten Ursprungs in Center und anschliessend drehen um phi
 * alle Punkte müssen in "normalen" Koordinaten gegeben sein
 * center: Center der Ellipse in deren System transformiert werden soll
 * phi: Winkel der Ellipse in deren System transformiert werden soll
 * */

Point Point::CoordTransToEllipse(const Point& center, double cphi, double sphi) const {
    Point p = Point(_x, _y);
    return (p - center).Rotate(cphi, -sphi);
}

/* Transformiert den Punkt aus dem Koordinatensystem der Ellipse
 * zurueck ins urspruengliche "normale" Koordinatensystem
 * center: Center der Ellipse aus deren System transformiert werden soll
 * phi: Winkel der Ellipse aus deren System transformiert werden soll
 * */
Point Point::CoordTransToCart(const Point& center, double cphi, double sphi) const {
    Point p = Point(_x, _y);
    return (p.Rotate(cphi, sphi) + center);
}

// rotiert Vektor um den Winkel theta
Point Point::Rotate(double ctheta, double stheta) const {
    return Point(_x * ctheta - _y*stheta, _x * stheta + _y * ctheta);
}

/*************************************************************
 überladene Operatoren
 ************************************************************/

// Addiert zwei Vektoren

const Point Point::operator+(const Point& p) const {
    return Point(_x + p.GetX(), _y + p.GetY());
}

// Subtrahiert zwei Vektoren

const Point Point::operator-(const Point& p) const {
    return Point(_x - p.GetX(), _y - p.GetY());
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool Point::operator==(const Point& p) const {
    return (fabs(_x - p.GetX()) < J_EPS && fabs(_y - p.GetY()) < J_EPS);
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool Point::operator!=(const Point& p) const {
    return (fabs(_x - p.GetX()) > J_EPS || fabs(_y - p.GetY()) > J_EPS);
}


// Multipliziert einen Vektor mit einem Skalar

const Point operator*(const Point& p, double f) {
    return Point(p.GetX() * f, p.GetY() * f);
}

// Dividiert einen Vektor durch einen Skalar

const Point operator/(const Point& p, double f) {
    return Point(p.GetX() / f, p.GetY() / f);
}
