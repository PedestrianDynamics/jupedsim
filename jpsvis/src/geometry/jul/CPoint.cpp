/* 
 * File:   CPoint.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 09:21
 */

#include "CPoint.h"

/************************************************************
  Konstruktoren
 ************************************************************/
CPoint::CPoint() {
    pX = 0.0;
    pY = 0.0;
}

CPoint::CPoint(float x, float y) {
    pX = x;
    pY = y;
}

CPoint::CPoint(const CPoint& orig) {
    pX = orig.GetX();
    pY = orig.GetY();
}

CPoint::~CPoint() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/

void CPoint::SetX(float x) {
    pX = x;
}

void CPoint::SetY(float y) {
    pY = y;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

float CPoint::GetX() const {
    return pX;
}

float CPoint::GetY() const {
    return pY;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/

// Norm des Vektors

float CPoint::Norm() const {
    return sqrt(pX * pX + pY * pY);
}

// gibt den normierten Vector zurueck

CPoint CPoint::Normalized() const {
    if (Norm() != 0)
        return (CPoint(pX, pY) / Norm());
    else return CPoint(0.0, 0.0);
}

// Skalarprodukt zweier Vektoren

float CPoint::ScalarP(const CPoint& v) const {
    return pX * v.GetX() + pY * v.GetY();
}

// rotiert Vektor um den Winkel theta

CPoint CPoint::Rotate(float theta) const {
    float s = sin(theta);
    float c = cos(theta);
    return CPoint(pX * c - pY*s, pX * s + pY * c);
}

// Transformiert die Koordinaten in Koordinaten der Ellipse
// dazu verschieben des Koordinaten Ursprungs in Center und anschliessend drehen um phi
// alle Pnktte müssen in "normale" Koordinaten gegeben sein

CPoint CPoint::CoordTransToEllipse(const CPoint& center, float phi) const {
    CPoint p = CPoint(pX, pY);
    return (p - center).Rotate(-phi);
}

// Transformiert zurueck ins urspruengliche Koord.system

CPoint CPoint::CoordTransToCart(const CPoint& center, float phi) const {
    CPoint p = CPoint(pX, pY);
    return (p.Rotate(phi) + center);
}

/*************************************************************
 überladene Operatoren
 ************************************************************/

// Addiert zwei Vektoren

const CPoint CPoint::operator+(const CPoint& p) const {
    return CPoint(pX + p.GetX(), pY + p.GetY());
}

// Subtrahiert zwei Vektoren

const CPoint CPoint::operator-(const CPoint& p) const {
    return CPoint(pX - p.GetX(), pY - p.GetY());
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool CPoint::operator==(const CPoint& p) const {
    return (pX == p.GetX() && pY == p.GetY());
}

// Vergleicht zwei Punkte/Vektoren komponentweise

bool CPoint::operator!=(const CPoint& p) const {
    return (pX != p.GetX() || pY != p.GetY());
}


// Multipliziert einen Vektor mit einem Skalar

CPoint operator*(const CPoint& p, float f) {
    return CPoint(p.GetX() * f, p.GetY() * f);
}

// Dividiert einen Vektor durch einen Skalar

CPoint operator/(const CPoint& p, float f) {
    return CPoint(p.GetX() / f, p.GetY() / f);
}
