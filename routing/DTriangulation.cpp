/*
 * DTriangulation.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: piccolo
 */

#include "DTriangulation.h"

using namespace std;

DTriangulation::DTriangulation()
{
     _cdt=NULL;
}

DTriangulation::~DTriangulation()
{

     for(unsigned int i = 0; i < _holesPolylines.size(); i++) {
          vector<p2t::Point*> poly = _holesPolylines[i];
          FreeClear(poly);
     }

     FreeClear(_outerConstraintsPolyline);
     delete _cdt;
}

void DTriangulation::Triangulate()
{

     _cdt= new p2t::CDT(_outerConstraintsPolyline);

     for(unsigned int h=0; h<_holesPolylines.size(); h++) {
          _cdt->AddHole(_holesPolylines[h]);
     }
     _cdt->Triangulate();
}

void DTriangulation::SetOuterPolygone(const std::vector<Point>& outPoly)
{

     for(unsigned int i=0; i<outPoly.size(); i++) {
          _outerConstraintsPolyline.push_back(new p2t::Point(outPoly[i]._x,outPoly[i]._y));
     }
}

void DTriangulation::AddHole(const std::vector<Point>& hole)
{

     std::vector<p2t::Point*> newHole;

     for(unsigned int i=0; i<hole.size(); i++) {
          newHole.push_back(new p2t::Point(hole[i]._x,hole[i]._y));
     }
     _holesPolylines.push_back(newHole);
}
