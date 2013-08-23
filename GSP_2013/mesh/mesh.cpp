/*
 * mesh.cpp
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

#include "mesh.h"
#include <iostream>

MeshNode::MeshNode(){
	_x=0.0;
	_y=0.0;
}
MeshNode::MeshNode(double x,double y){
	_x=x;
	_y=y;
}

MeshEdge::MeshEdge(int n1,int n2,int c1, int c2){
	_n1=n1;
	_n2=n2;
	_c1=c1;
	_c2=c2;
}

MeshCell::MeshCell(double midx,double midy,std::vector<int> node_id,
			 double normvec[3],std::vector<int> edge_id,
			 std::vector<int> wall_id){
	_midx=midx;
	_midy=midy;
	_node_id=node_id;
	for(int i=0;i<3;i++)
		_normvec[i]=normvec[i];
	_edge_id=edge_id;
	_wall_id=wall_id;
}

MeshCellGroup::MeshCellGroup(std::string groupname,std::vector<MeshCell*> cells){
	_groupname=groupname;
	_cells=cells;
}
MeshCellGroup::~MeshCellGroup(){
	for(unsigned int i=0;i<_cells.size();i++)
			delete _cells[i];
}
std::vector<MeshCell*> MeshCellGroup::get_cells(){
	return _cells;
}

MeshData::MeshData(){
	_mNodes=std::vector<MeshNode*>();
	_mEdges=std::vector<MeshEdge*>();
	_mOutEdges=std::vector<MeshEdge*>();
	_mCellGroups=std::vector<MeshCellGroup*>();
}
MeshData::~MeshData(){
	std::cout<<"Meshdata wird zerstoert"<<std::endl;
	for(unsigned int i=0;i<_mNodes.size();i++)
		delete _mNodes[i];
	for(unsigned int i=0;i<_mEdges.size();i++)
			delete _mEdges[i];
	for(unsigned int i=0;i<_mOutEdges.size();i++)
			delete _mOutEdges[i];
	for(unsigned int i=0;i<_mCellGroups.size();i++)
			delete _mCellGroups[i];
}

MeshData::MeshData(std::vector<MeshNode*> mn,std::vector<MeshEdge*> me,
		std::vector<MeshEdge*> moe,std::vector<MeshCellGroup*> mcg){
	_mNodes=mn;
	_mEdges=me;
	_mOutEdges=moe;
	_mCellGroups=mcg;
}

/*
std::istream& operator>>(std::istream& is, MeshNode& mn){
	is>>mn._x>>mn._y;
	return is;
}
*/
