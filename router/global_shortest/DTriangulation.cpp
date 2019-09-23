/**
 * \file        DTriangulation.cpp
 * \date        Nov 30, 2012
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#include "DTriangulation.h"

DTriangulation::DTriangulation()
{
     _cdt=nullptr;
}

DTriangulation::~DTriangulation()
{

     for(unsigned int i = 0; i < _holesPolylines.size(); i++) {
          std::vector<p2t::Point*> poly = _holesPolylines[i];
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
