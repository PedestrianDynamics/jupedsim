/*
 * NavMesh.h
 *
 *  Created on: Oct 29, 2012
 *      Author: Ulrich Kemloh
 */

#ifndef NAVMESH_H_
#define NAVMESH_H_


//#define _CGAL

// CGAL libs
#ifdef _CGAL

#include "ConvexDecomp.h"
#include "Triangulation.h"

//#include <CGAL/basic.h>
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Partition_traits_2.h>
//#include <CGAL/Partition_is_valid_traits_2.h>
//#include <CGAL/polygon_function_objects.h>
//#include <CGAL/partition_2.h>
//#include <CGAL/point_generators_2.h>
//#include <CGAL/random_polygon_2.h>
//#include <cassert>
//#include <list>
//
//#include <CGAL/Cartesian.h>
//#include <CGAL/centroid.h>
//
//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//typedef CGAL::Partition_traits_2<K> Traits;
//typedef CGAL::Is_convex_2<Traits> Is_convex_2;
//typedef Traits::Polygon_2 Polygon_2;
//typedef Traits::Point_2 Point_2;
//
//
//typedef Polygon_2::Vertex_const_iterator Vertex_iterator;
//typedef Polygon_2::Vertex_const_circulator Vertex_circulator;
//typedef Polygon_2::Edge_const_iterator Edge_iterator;
//typedef std::list<Polygon_2> Polygon_list;
//typedef Polygon_list::const_iterator Polygon_iterator;
//typedef CGAL::Partition_is_valid_traits_2<Traits, Is_convex_2> Validity_traits;

#endif

#include "../geometry/Point.h"
#include "cmath"
#include "../geometry/Building.h"
#include <string>


class NavMesh {

	class Vertex {
	public:
		Point pPos;
		int id;
		bool operator==(const  Vertex& v) const {
			return v.pPos==pPos;
		}
		bool operator< (const Vertex& v) const{
			return v.pPos.pX<pPos.pX;
		}
		bool operator> (const Vertex& v) const{
			return v.pPos.pX>pPos.pX;
		}
	};

	class Node {
	public:
		std::string pGroup;
		Point pCentroid;
		int id;
		std::vector<Vertex> pHull;
		std::vector<int> pObstacles;
		std::vector<int> pPortals;
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
				if(fabs(det)<J_EPS) det=0.0;

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
			if(fabs(det)<J_EPS) det=0.0;

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
		Edge(){
			id=-1;
			pNode0=-1;
			pNode1=-1;
		}
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
	void WriteToFileTraVisTo(std::string fileName, const std::vector<Point>& points);

	int AddVertex(Vertex* v);
	int AddEdge(Edge* e);
	int AddObst(Obstacle* o);
	int AddNode(Node* n);
	///return the vertex with the corresponding point
	Vertex* GetVertex(const Point& p);
	void DumpNode(int id);

private:
	std::vector<Vertex*> pVertices;
	std::vector<Edge*> pEdges;
	std::vector<Obstacle*> pObst;
	std::vector<Node*> pNodes;
	Building* pBuilding;

	// convexify the created nav mesh
	void Convexify();

	//add the additional surrounding world obstacle
	void Finalize();

	/// return the id of the edge
	int IsPortal(Point& p1, Point&  p2);
	/// return the id of the obstacle
	int IsObstacle(Point& p1, Point&  p2);


	std::vector<Node*> new_nodes;
	vector<int> problem_nodes;
};

#endif /* NAVMESH_H_ */
