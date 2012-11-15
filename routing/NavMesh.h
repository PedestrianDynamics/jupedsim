/*
 * NavMesh.h
 *
 *  Created on: Oct 29, 2012
 *      Author: piccolo
 */

#ifndef NAVMESH_H_
#define NAVMESH_H_

#include "../geometry/Point.h"
#include "cmath"
#include "../geometry/Building.h"
#include <string>

class NavMesh {

	class Vertex {
	public:
		Point pPos;
		int id;
	};

	class Node {
	public:
		std::string pGroup;
		Point pCentroid;
		int id;
		std::vector<Vertex> pHull;
		std::vector<int> pObstacles;
		double pNormalVec[3];

		bool operator()(Node*a ,Node* b){
			return a->pGroup < b->pGroup;
		}

		///http://stackoverflow.com/questions/471962/how-do-determine-if-a-polygon-is-complex-convex-nonconvex
		bool IsConvex(){

			unsigned int hsize=pHull.size();
			unsigned int pos=0;
			unsigned int neg=0;

			for(unsigned int i=0;i<hsize;i++)
			{
				Point vecAB= pHull[(i+1)%hsize].pPos-pHull[i%hsize].pPos;
				Point vecBC= pHull[(i+2)%hsize].pPos-pHull[(i+1)%hsize].pPos;
				double det= vecAB.Det(vecBC);
				if(fabs(det)<EPS) det=0.0;

				if(det<0.0){
					neg++;
				}
				else if(det>0.0)
				{
					pos++;
				}
				else
				{
					pos++;
					neg++;
				}

			}

			if ( (pos==hsize ) || (neg==hsize) ) {
				return true;
			}
			return false;
		}

		///http://stackoverflow.com/questions/9473570/polygon-vertices-clockwise-or-counterclockwise/
		bool IsClockwise(){
			if(pHull.size()<3){
				std::cerr<<"You need at least 3 vertices. Node ID ["<<id<<" ]"<<endl;
				exit(EXIT_FAILURE);
			}

			Point vecAB= pHull[1].pPos-pHull[0].pPos;
			Point vecBC= pHull[2].pPos-pHull[1].pPos;

			double det=vecAB.Det(vecBC);
			if(fabs(det)<EPS) det=0.0;

			return ( det<=0.0 );
		}


	};

	class Edge {
	public:
		int id;
		Vertex pStart;
		Vertex pEnd;
		//Point pDisp;
		int pNode0;
		int pNode1;
	};

	class Obstacle {
	public:
		int id;
		Vertex pStart;
		Vertex pEnd;
		//Point pDisp;
		int pNode0;
		int pNextObst;

		int GetCommonVertex(Obstacle* obst){
			if(obst->pEnd.id==pEnd.id) return pEnd.id;
			if(obst->pEnd.id==pStart.id) return pStart.id;
			if(obst->pStart.id==pStart.id) return pStart.id;
			if(obst->pStart.id==pEnd.id) return pEnd.id;
			return -1;
		}
	};



	template<typename A>
	bool IsElementInVector(const std::vector<A> &vec, A& el) {
		typename std::vector<A>::const_iterator it;
		it = find (vec.begin(), vec.end(), el);
		if(it==vec.end()){
			return false;
		}else{
			return true;
		}
	}

public:
	NavMesh(Building* b);
	virtual ~NavMesh();
	void BuildNavMesh();
	void WriteToFile(std::string fileName);
	void WriteToFileTraVisTo(std::string fileName);
	void AddVertex(Vertex* v);
	void AddEdge(Edge* e);
	void AddObst(Obstacle* o);
	void AddNode(Node* n);
	///return the vertex with the corresponding point
	Vertex* GetVertex(const Point& p);
	void DumpNode(int id);

private:
	std::vector<Vertex*> pVertices;
	std::vector<Edge*> pEdges;
	std::vector<Obstacle*> pObst;
	std::vector<Node*> pNodes;
	Building* pBuilding;

	void Convexify();



};

#endif /* NAVMESH_H_ */
