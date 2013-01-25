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
	double dx = j._x - i._x;
	double dy = j._y - i._y;
	return dx * dx + dy * dy;
}

double CGeomVector::distance(Point i, Point j)
{
//	return sqrt(distance(i, j));
	double dx = j._x - i._x;
	double dy = j._y - i._y;
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
	return (q._x - p._x) * (r._y - p._y) - (q._y - p._y) * (r._x - p._x);
}

CGeomHomog CGeomVector::meet(Point i, Point j)
{
	return CGeomHomog(i._x * j._y - i._y * j._x, i._y - j._y, j._x - i._x);
}
