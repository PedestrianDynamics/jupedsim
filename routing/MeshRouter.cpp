/*
 * MeshRouter.cpp
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

#include "MeshRouter.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"


MeshRouter::MeshRouter() {
	_building=NULL;
	_meshdata=MeshData();
}

MeshRouter::~MeshRouter() {

}

int MeshRouter::FindExit(Pedestrian* p) {
	p->SetExitIndex(1);
	//p->SetExitLine(_building->getGetCrossing(0));
	return 1;
}

void MeshRouter::Init(Building* b) {
	_building=b;
	Log->Write("ERROR: \tdo not use this  <<Mesh>> router !!");

	std::ifstream meshfile;
	meshfile.open("../GSP_2013/test.nav", std::ifstream::in);
	if(!meshfile.is_open()){
		Log->Write("ERROR: \tcould not open meshfile");
	}
	std::vector<MeshNode*> nodes;
    unsigned int countNodes=0;
	meshfile>>countNodes;
	for(unsigned int i=0;i<countNodes;i++){
		double temp1,temp2;
		meshfile>>temp1>>temp2;
		nodes.push_back(new MeshNode(temp1,temp2));
	}
	std::vector<MeshEdge*> edges;
	unsigned int countEdges=0;
	meshfile>>countEdges;
	for(unsigned int i=0;i<countEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		edges.push_back(new MeshEdge(t1,t2,t3,t4));
	}
	std::vector<MeshEdge*> outedges;
	unsigned int countOutEdges=0;
	meshfile>>countOutEdges;
	for(unsigned int i=0;i<countOutEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		outedges.push_back(new MeshEdge(t1,t2,t3,t4));
	}

	std::vector<MeshCellGroup*> mCellGroups;
	char tmp[256];
	meshfile.getline(tmp,256);
	while(std::string(tmp).size()>1){
		std::string groupname=tmp;
		unsigned int countCells=0;
		meshfile>>countCells;
		std::vector<MeshCell*> mCells;
		for(unsigned int i=0;i<countCells;i++){
			double midx,midy;
			meshfile>>midx>>midy;
			unsigned int countNodes=0;
			meshfile>>countNodes;
			std::vector<int> node_id;
			for(unsigned int j=0;j<countNodes;j++){
				int tmp;
				meshfile>>tmp;
				node_id.push_back(tmp);
			}
			double* normvec=new double[3];
			for (unsigned int j=0;j<3;j++){
				int tmp;
				meshfile>>tmp;
				normvec[i]=tmp;
			}
			unsigned int countEdges=0;
			meshfile>>countEdges;
			std::vector<int> edge_id;
			for(unsigned int j=0;j<countEdges;j++){
				int tmp;
				meshfile>>tmp;
				edge_id.push_back(tmp);
			}
			unsigned int countWalls=0;
			meshfile>>countWalls;
			std::vector<int> wall_id;
			for(unsigned int j=0;j<countWalls;j++){
				int tmp;
				meshfile>>tmp;
				wall_id.push_back(tmp);
			}

			mCells.push_back(new MeshCell(midx,midy,node_id,normvec,edge_id,wall_id));
		}
		mCellGroups.push_back(new MeshCellGroup(groupname,mCells));
	}
	MeshData meshdat(nodes,edges,outedges,mCellGroups);
	_meshdata=meshdat;
}



