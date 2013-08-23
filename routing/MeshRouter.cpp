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
	//TODO: umstellung auf Zeigern
	_meshdata=MeshData();
}

MeshRouter::~MeshRouter() {
	delete &_meshdata;
}

int MeshRouter::FindExit(Pedestrian* p) {
	std::cout<<"calling the mesh router"<<std::endl;
	//return any transition or crossing in the actual room.
	p->SetExitIndex(-1);
	//find a proper navigation line
//	p->SetExitLine(_building->GetAllTransitions()[0]);
//	p->SetExitLine(_building->GetTransition(0)GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllCrossings()[0]);
	return -1;
}

void MeshRouter::Init(Building* b) {
	_building=b;
	Log->Write("ERROR: \tdo not use this  <<Mesh>>  router !!");

	std::ifstream meshfiled;
	meshfiled.open("../GSP_2013/test.nav", std::ios::in);
	if(!meshfiled.is_open()){
		Log->Write("ERROR: \tcould not open meshfile");
	}
	std::stringstream meshfile;
	meshfile<<meshfiled.rdbuf();
	meshfiled.close();

	std::vector<MeshNode*> nodes;
	std::vector<MeshEdge*> edges;
	std::vector<MeshEdge*> outedges;
	std::vector<MeshCellGroup*> mCellGroups;



    unsigned int countNodes=0;
	meshfile>>countNodes;
	for(unsigned int i=0;i<countNodes;i++){
		double temp1,temp2;
		meshfile>>temp1>>temp2;
		nodes.push_back(new MeshNode(temp1,temp2));
	}
	std::cout<<"Read "<<nodes.size()<<" Nodes from file"<<std::endl;


	unsigned int countEdges=0;
	meshfile>>countEdges;
	for(unsigned int i=0;i<countEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		edges.push_back(new MeshEdge(t1,t2,t3,t4));
	}
	std::cout<<"Read "<<edges.size()<<" inner Edges from file"<<std::endl;


	unsigned int countOutEdges=0;
	meshfile>>countOutEdges;
	for(unsigned int i=0;i<countOutEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		outedges.push_back(new MeshEdge(t1,t2,t3,t4));
	}
	std::cout<<"Read "<<outedges.size()<<" outer Edges from file"<<std::endl;
	/***/
	//std::cout<<outedges[0]->get_n1()<<" "<<outedges[1]->get_c2()<<std::endl;



	/*char tmp[256];
	meshfile.getline(tmp,256);
	std::string tmp;
	meshfile.ignore(2);
	std::getline(meshfile,tmp);
	std::cout<<"<"<<std::string(tmp)<<">"<<std::endl;
	int stop=0;*/
	/*
	while(!meshfile.eof()){
		//std::cout<<"here"<<std::endl;
		std::string groupname;
		bool  namefound=false;
		while(!namefound && getline(meshfile,groupname)){
			if (groupname.size()>2){
				namefound=true;
			}
		}
		std::cout<<"<"<<groupname<<">"<<std::endl;
		//if(!(meshfile>>groupname)){
		//	Log->Write("Konnte Gruppennamen der Zellen nicht lesen");
		//}
		unsigned int countCells=0;
		meshfile>>countCells;
		std::cout<<countCells<<std::endl;
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
			std::cout<<midx<<std::endl;
			mCells.push_back(new MeshCell(midx,midy,node_id,normvec,edge_id,wall_id));
			std::cout<<mCells.back()->get_midx()<<std::endl;
		}
		mCellGroups.push_back(new MeshCellGroup(groupname,mCells));
		std::cout<<mCellGroups.back()->get_cells().back()->get_midx()<<std::endl;
	}
	std::cout<<"Ende while"<<std::endl;
    */
	//std::cout<<"here"<<std::endl;
	//MeshData meshdat(nodes,edges,outedges,mCellGroups);
	//_meshdata=meshdat;
	//MeshData meshdat(nodes,edges,outedges,mCellGroups);
	std::cout<<"here"<<std::endl;
	_meshdata=*(new MeshData(nodes,edges,outedges,mCellGroups));
	std::cout<<"here"<<std::endl;
}



