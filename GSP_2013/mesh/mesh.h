/*
 * mesh.h
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

#ifndef MESH_H_
#define MESH_H_

#include <fstream>
#include <vector>

class MeshNode;
class MeshEdge;
class MeshCellGroup;

class MeshData{
public:
	MeshData();
	MeshData(std::vector<MeshNode*>,std::vector<MeshEdge*>,
			std::vector<MeshEdge*>,std::vector<MeshCellGroup*>);
private:
	std::vector<MeshNode*> _mNodes;
	std::vector<MeshEdge*> _mEdges;
	std::vector<MeshEdge*> _mOutEdges;
	std::vector<MeshCellGroup*> _mCellGroups;

};

class MeshNode{
public:
	MeshNode();
	MeshNode(double,double);
	double get_x();
	double get_y();
	//friend std::istream& operator>>(std::istream& is, MeshNode& mn);
private:
	double _x;
	double _y;
};

class MeshEdge{
public:
	MeshEdge(int,int,int,int);
	//friend std::istream& operator>>(std::istream& is, MeshEdge& mn);
private:
	int _n1;
	int _n2;
	int _c1;
    int _c2;
};

class MeshCell{
public:
	MeshCell(double,double,std::vector<int>,
			 double*,std::vector<int>,std::vector<int>);
private:
	double _midx;
	double _midy;
	std::vector<int> _node_id;
	double _normvec[3];
	std::vector<int> _edge_id;
	std::vector<int> _wall_id;
};

class MeshCellGroup{
public:
	MeshCellGroup(std::string,std::vector<MeshCell*>);
private:
    std::string _groupname;
    std::vector<MeshCell*> _cells;
};

#endif /* MESH_H_ */
