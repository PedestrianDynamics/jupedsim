/* 
 * File:   CPoint.h
 * Author: andrea
 *
 * Created on 30. September 2010, 09:21
 */

#ifndef _POINT_H
#define	_POINT_H

#include <cmath>

class CPoint {
private:
    float pX;
    float pY;

public:
    CPoint();
    CPoint(float x, float y);
    CPoint(const CPoint& orig);
    virtual ~CPoint();

    // Setter-Funktionen
    void SetX(float x); // setzt x-Koordinate
    void SetY(float y); // setzt y-Koordinate

    // Getter-Funktionen
    float GetX() const; // gibt x-Koordinate zurück
    float GetY() const; // gibt y-Koordinate zurück

    // Sonstiges
    float Norm() const; // Norm des Vektors
    CPoint Normalized() const;
    float ScalarP(const CPoint& v) const; // Skalarprodukt zweier Vektoren
    CPoint Rotate(float theta) const; // rotiert Vektor um den Winkel theta
    CPoint CoordTransToEllipse(const CPoint& center, float phi) const; // Verschiebung und Drehung
    CPoint CoordTransToCart(const CPoint& center, float phi) const; // Verschiebung und Drehung

    const CPoint operator+(const CPoint& p) const; // Addiert zwei Vektoren
    const CPoint operator-(const CPoint& p) const; // Subtrahiert zwei Vektoren
    bool operator==(const CPoint& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise
    bool operator!=(const CPoint& p) const; // Vergleicht zwei Punkte/Vektoren komponentweise
};


// Operatoren mit float
CPoint operator*(const CPoint& p, const float f);
CPoint operator/(const CPoint& p, const float f);

#endif	/* _POINT_H */

