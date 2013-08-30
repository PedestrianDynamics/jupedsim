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
#include "../../geometry/Point.h"
#include "../../geometry/Line.h"

class MeshEdge;
class MeshCell;
class MeshCellGroup;

class MeshData{
public:
	MeshData();
	~MeshData();
	MeshData(std::vector<Point*>,std::vector<MeshEdge*>,
			std::vector<MeshEdge*>,std::vector<MeshCellGroup*>);
	std::vector<Point*> get_nodes(){return _mNodes;}
	std::vector<MeshEdge*> get_edges(){return _mEdges;}
	std::vector<MeshEdge*> get_outEdges(){return _mOutEdges;}
	std::vector<MeshCellGroup*> get_cellGroups(){return _mCellGroups;}
	unsigned int get_cellCount(){return _mCellCount;};

	MeshCell* getCellAtPos(unsigned int tpos);

	MeshCell* findCell(Point testp,int& cell_id);

private:
	std::vector<Point*> _mNodes;
	std::vector<MeshEdge*> _mEdges;
	std::vector<MeshEdge*> _mOutEdges;
	std::vector<MeshCellGroup*> _mCellGroups;
	unsigned int _mCellCount;

};

class MeshEdge:public Line{
public:
	MeshEdge(int,int,int,int,Point p1=Point(),Point p2=Point());//:Line(p1,p2);
	int get_n1(){return _n1;};
	int get_n2(){return _n2;};
	int get_c1(){return _c1;};
	int get_c2(){return _c2;};
	//friend std::istream& operator>>(std::istream& is, MeshEdge& mn);
private:
	int _n1; //ID of Node 1
	int _n2; //ID of Node 2
	int _c1; //ID of Cell 1
    int _c2; //ID of Cell 2
};

class MeshCell{
public:
	MeshCell(double,double,std::vector<int>,
			 double*,std::vector<int>,std::vector<int>,int);
	~MeshCell();
	//double get_midx(){return _midx;};
	//double get_midy(){return _midy;};
	Point get_mid(){return _mid;};
	std::vector<int> get_nodes(){return _node_id;};
	std::vector<int> get_edges(){return _edge_id;};
	int get_id(){return _tc_id;};
private:
	//double _midx;
	//double _midy;
	Point _mid;
	std::vector<int> _node_id;
	//double *_normvec;
	double _normvec[3];
	std::vector<int> _edge_id;
	std::vector<int> _wall_id;
	int _tc_id;//Cell ID unique for all cells in building
};

class MeshCellGroup{
public:
	MeshCellGroup(std::string,std::vector<MeshCell*>);
	~MeshCellGroup();
	std::vector<MeshCell*> get_cells();
private:
    std::string _groupname;
    std::vector<MeshCell*> _cells;
};

unsigned int calc_CellCount(std::vector<MeshCellGroup*> mcg);

#endif /* MESH_H_ */
