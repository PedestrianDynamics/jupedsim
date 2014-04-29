/*
 * MeshRouter.h
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

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
