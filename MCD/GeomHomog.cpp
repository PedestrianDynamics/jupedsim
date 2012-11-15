// GeomHomog.cpp: implementation of the CGeomHomog class.
//
//////////////////////////////////////////////////////////////////////

#include "GeomHomog.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeomHomog::CGeomHomog()
{
	w = 1.0;
	x = 0.0;
	y = 0.0;
}

CGeomHomog::CGeomHomog(double pw, double px, double py)
{
	w = pw; 
	x = px; 
	y = py;
}

CGeomHomog::CGeomHomog(Point p)
{
	CGeomHomog(1.0, p.pX, p.pY);
}

CGeomHomog::CGeomHomog(int px, int py) 
{
	CGeomHomog(1.0, px, -py);
}

CGeomHomog::CGeomHomog(const CGeomHomog& homog)
{
	w = homog.w;
	x = homog.x;
	y = homog.y;
}


CGeomHomog::~CGeomHomog()
{

}

CGeomHomog& CGeomHomog::add(CGeomHomog& p)
{
	x = x + p.x; 
	y = y + p.y;
	return *this;
}


bool CGeomHomog::left(Point p)
{
	return dot(p) > 0;
}

bool CGeomHomog::right(Point p)
{
	return dot(p) < 0;
}

double CGeomHomog::dot(Point p)
{
	return w + x * p.pX + y * p.pY;
}

double CGeomHomog::dot(CGeomHomog h)
{
	return w * h.w + x * h.x + y * h.y;
}

bool CGeomHomog::cw(CGeomHomog p, CGeomHomog q, CGeomHomog r)
{
	return det(p, q, r) < 0;
}

bool CGeomHomog::ccw(CGeomHomog p, CGeomHomog q, CGeomHomog r)
{
	return det(p, q, r) > 0;
}

double CGeomHomog::det(CGeomHomog p, CGeomHomog q, CGeomHomog r)
{
	return p.w * q.perpdot(r) - q.w * p.perpdot(r) + r.w * p.perpdot(q);
}

double CGeomHomog::perpdot(CGeomHomog &p)
{
	return x * p.y - y * p.x;
}

CGeomHomog& CGeomHomog::neg()
{
	w = -w;
	x = -x;
	y = -y;
	return *this;
}

void CGeomHomog::meet(Point i, Point j)
{
	w = i.pX * j.pY - i.pY * j.pX;
	x = i.pY - j.pY;
	y = j.pX - i.pX;
}

void CGeomHomog::meet(Point p)
{
	double tempW, tempX, tempY;
	tempW = x * p.pY - y * p.pX;
	tempX = y - w * p.pY;
	tempY = w * p.pX - x;
	w = tempW;
	x = tempX;
	y = tempY;
}

void CGeomHomog::meet(CGeomHomog p)
{
	double tempW, tempX, tempY;
	tempW = x * p.y - y * p.x;
	tempX = p.w * y - w * p.y;
	tempY = w * p.x - p.w * x;
	w = tempW;
	x = tempX;
	y = tempY;
}

Point CGeomHomog::toPoint()
{
	return Point((int)x/w, (int)y/w);
}
