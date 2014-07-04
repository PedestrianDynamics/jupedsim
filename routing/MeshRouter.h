/**
 * \file        MeshRouter.h
 * \date        Aug 21, 2013
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#ifndef MESHROUTER_H_
#define MESHROUTER_H_

#include "Router.h"
#include "mesh/Mesh.h"

class MeshRouter: public Router {
private:
     Building* _building;
     MeshData* _meshdata;

     std::vector<MeshEdge*> AStar(Pedestrian* p,int& status)const;
     NavLine Funnel(Point&,Point&,std::vector<MeshEdge*>)const;
     NavLine FunnelRad(Point&,Point&,std::vector<MeshEdge*>);
     MeshEdge* Visibility(Point&,Point&,std::vector<MeshEdge*>)const;
     std::string GetMeshFileName() const;
     void FixMeshEdges();
public:
     MeshRouter();
     virtual ~MeshRouter();

     virtual int FindExit(Pedestrian* p);
     virtual void Init(Building* b);

};

#endif /* MESHROUTER_H_ */
