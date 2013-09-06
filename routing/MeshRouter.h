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

	int GetNextEdge(Pedestrian* p, MeshEdge** edge);
	std::vector<MeshEdge*> AStar(Pedestrian* p);
	NavLine Funnel(Point&,Point&,std::vector<MeshEdge*>);
public:
	MeshRouter();
	virtual ~MeshRouter();

	virtual int FindExit(Pedestrian* p);
	virtual void Init(Building* b);

};

#endif /* MESHROUTER_H_ */
