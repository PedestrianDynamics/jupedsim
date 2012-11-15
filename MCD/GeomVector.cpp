// GeomVector.cpp: implementation of the CGeomVector class.
//
//////////////////////////////////////////////////////////////////////

#include "GeomVector.h"
#include "GeomHomog.h"

#include "cmath"
#include "../geometry/Point.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeomVector::CGeomVector()
{

}

CGeomVector::~CGeomVector()
{

}

double CGeomVector::distance2(Point i, Point j)
{
	double dx = j.pX - i.pX;
	double dy = j.pY - i.pY;
	return dx * dx + dy * dy;
}

double CGeomVector::distance(Point i, Point j)
{
//	return sqrt(distance(i, j));
	double dx = j.pX - i.pX;
	double dy = j.pY - i.pY;
	return sqrt(dx * dx + dy * dy);
}

bool  CGeomVector::left(Point p, Point q, Point r)
{
	return delta3(p, q, r) > 0.0? true: false;
}

bool  CGeomVector::on(Point p, Point q, Point r)
{
	return delta3(p, q, r) == 0.0? true: false;
}

bool  CGeomVector::right(Point p, Point q, Point r)
{
	return delta3(p, q, r) < 0.0? true: false;
}

double CGeomVector::delta3(Point p, Point q, Point r)
{
	return (q.pX - p.pX) * (r.pY - p.pY) - (q.pY - p.pY) * (r.pX - p.pX);
}

CGeomHomog CGeomVector::meet(Point i, Point j)
{
	return CGeomHomog(i.pX * j.pY - i.pY * j.pX, i.pY - j.pY, j.pX - i.pX);
}
