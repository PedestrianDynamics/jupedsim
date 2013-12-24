/* 
 * File:   Line.cpp
 * Author: andrea
 * 
 * Created on 30. September 2010, 09:40
 */

#include "Line.h"

/************************************************************
  Konstruktoren
 ************************************************************/
Line::Line() {
    SetPoint1(CPoint()); //Defaul-Constructor ist (0.0,0.0)
    SetPoint2(CPoint());
}

Line::Line(const CPoint& p1, const CPoint& p2) {
    SetPoint1(p1);
    SetPoint2(p2);
}

Line::Line(const Line& orig) {
    pPoint1 = orig.GetPoint1();
    pPoint2 = orig.GetPoint2();
}

Line::~Line() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Line::SetPoint1(const CPoint& p) {
    pPoint1 = p;
}

void Line::SetPoint2(const CPoint& p) {
    pPoint2 = p;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/
const CPoint& Line::GetPoint1(void) const {
    return pPoint1;
}

const CPoint& Line::GetPoint2(void) const {
    return pPoint2;
}

/*************************************************************
 Sonstige Funktionen
 ************************************************************/
CPoint Line::NormalVec() const {
    float nx, ny, norm;
    CPoint r = GetPoint2() - GetPoint1();

    if (r.GetX() == 0.0) {
        nx = 1;
        ny = 0;
    } else {
        nx = -r.GetY() / r.GetX();
        ny = 1;
        /* Normieren */
        norm = sqrt(nx * nx + ny * ny);
        nx /= norm;
        ny /= norm;
    }
    return CPoint(nx, ny);
}

float Line::NormalComp(const CPoint& v) const {
    CPoint diff = GetPoint2() - GetPoint1();
    CPoint u, x;
    float nu = diff.Norm();

    if (nu < 0.0001) {
        printf("ERROR: \tCLine::NormalComp(): normal_vector  length==0\nstop\n");
        exit(0);
    }
    u = diff / nu;

    if (u.GetX() < 0.0001)
        x = CPoint(1.0, 0.0);
    else
        x = CPoint(-u.GetY() / u.GetX(), 1.0);

    return fabs(v.ScalarP(x)); //sqrt(v.x*v.x+v.y*v.y); //
}

CPoint Line::ShortestPoint(const CPoint& p) const {
    CPoint r = GetPoint1();
    CPoint s = GetPoint2();
    CPoint t = r - s;
    CPoint tmp;
    float lambda;

    tmp = p - s;
    lambda = tmp.ScalarP(t) / t.ScalarP(t);
    CPoint f = s + t*lambda;

    /* Prüfen ob Punkt in der Linie,sonst entsprechenden Eckpunkt zurückgeben */
    if (lambda < 0)
        f = s;
    if (lambda > 1)
        f = r;

    return f;
}

bool Line::IsInLine(const CPoint& p) const {
    float ax, ay, bx, by, px, py;
    CPoint a = GetPoint1();
    CPoint b = GetPoint2();
    float lambda;
    ax = a.GetX();
    ay = a.GetY();
    bx = b.GetX();
    by = b.GetY();
    px = p.GetX();
    py = p.GetY();
    if (ax != bx)
        lambda = (px - ax) / (bx - ax);
    else if (ay != by)
        lambda = (py - ay) / (by - ay);
    else {
        printf("ERROR: \tIsInLine: Endpunkt = Startpunkt!!!");
        exit(0);
    }
    return (0 <= lambda) && (lambda <= 1);
}

float Line::DistTo(const CPoint& p) const {
    return (p - ShortestPoint(p)).Norm();
}

/*************************************************************
 Ausgabe Funktionen
 ************************************************************/

