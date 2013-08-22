/*
 * mesh.cpp
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

#include "mesh.h"

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

MeshData::MeshData(){
	_mNodes=std::vector<MeshNode*>();
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
