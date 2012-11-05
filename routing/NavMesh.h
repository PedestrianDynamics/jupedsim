/*
 * NavMesh.h
 *
 *  Created on: Oct 29, 2012
 *      Author: piccolo
 */

#ifndef NAVMESH_H_
#define NAVMESH_H_

#include "../geometry/Point.h"
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
			cout<<"calling"<<endl;
			return a->pGroup < b->pGroup;
		}

//		bool operator<(const Node& other) const {
//			cout<<"calling"<<endl;
//			return pGroup < other.pGroup;
//		}
//
//		bool compare(const Node* other) const {
//			cout<<"calling"<<endl;
//			return pGroup < other->pGroup;
//		}
	};

	class Edge {
	public:
		int id;
		Vertex pStart;
		Point pDisp;
		int pNode0;
		int pNode1;
	};

	class Obst {
	public:
		int id;
		Vertex pStart;
		Point pDisp;
		int pNode0;
		int pNextObst;
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
	NavMesh();
	virtual ~NavMesh();
	void BuildNavMesh(Building* b);
	void WriteToFile(std::string fileName);
	void WriteToFileTraVisTo(std::string fileName);
	void AddVertex(Vertex* v);
	void AddEdge(Edge* e);
	void AddObst(Obst* o);
	void AddNode(Node* n);
	///return the vertex with the corresponding point
	Vertex* GetVertex(const Point& p);

private:
	std::vector<Vertex*> pVertices;
	std::vector<Edge*> pEdges;
	std::vector<Obst*> pObst;
	std::vector<Node*> pNodes;
	Building* pBuilding;



};

#endif /* NAVMESH_H_ */
