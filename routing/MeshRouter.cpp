/*
 * MeshRouter.cpp
 *
 *  Created on: 21.08.2013
 *      Author: dominik
 */

#include <iomanip>

#include "MeshRouter.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"

using namespace std;

MeshRouter::MeshRouter() {
	_building=NULL;
	_meshdata=NULL;
}

MeshRouter::~MeshRouter() {
	delete _meshdata;
}

// Debug
void astar_print(bool* closedlist,bool* inopenlist,int* predlist,
	unsigned int c_totalcount,int act_id,std::vector<std::pair<double,MeshCell*> >openlist){
	std::cout<<"----------------------------------"<<std::endl;
	std::cout<<"act_id: "<<act_id<<std::endl;
	std::cout<<"Closed-List"<<std::endl;
	for(unsigned int i=0;i<c_totalcount;i++)
		std::cout<<(closedlist[i]?"1 ":"0 ");
	std::cout<<std::endl;
	std::cout<<"Inopen-List"<<std::endl;
		for(unsigned int i=0;i<c_totalcount;i++)
			std::cout<<(inopenlist[i]?"1 ":"0 ");
		std::cout<<std::endl;
	std::cout<<"Predecessor-List"<<std::endl;
	for(unsigned int i=0;i<c_totalcount;i++)
		(predlist[i]!=-1?std::cout<<predlist[i]<<" ":std::cout<<"* ");
	std::cout<<std::endl;
	std::cout<<"Openlist"<<std::endl;
	for(unsigned int i=0;i<openlist.size();i++)
		std::cout<<openlist.at(i).second->get_id()<<"(f="<<openlist.at(i).first<<") ";
	std::cout<<std::endl;
	std::cout<<"----------------------------------"<<std::endl;
}

void print_path(int* predlist,int c_start_id,int c_goal_id){
	std::cout<<"reverse path:"<<std::endl;
	int act_id=c_goal_id;
	while(act_id!=c_start_id){
		std::cout<<act_id<<" "<<std::endl;
		act_id=predlist[act_id];
	}
	std::cout<<c_start_id<<std::endl;
}

int MeshRouter::GetNextEdge(Pedestrian* p, MeshEdge** edge){
	/*
			 * A* TEST IMPLEMENTATION
			 */

			int c_start_id;
			//Point testp_start(0,0);
			Point  testp_start=p->GetPos();
			//std::cout<<testp_start.toString()<<std::endl;;
			MeshCell* start_cell=_meshdata->findCell(testp_start,c_start_id);
			if(start_cell!=NULL){
				//std::cout<<testp_start.toString()<<"Gefunden in Zelle: "<<c_start_id<<std::endl;
			}
			else{
				std::cout<<"Nicht gefunden"<<std::endl;
			}
			int c_goal_id;
			Point testp_goal(-6,17);
			MeshCell* goal_cell=_meshdata->findCell(testp_goal,c_goal_id);
			if(goal_cell!=NULL){
				//std::cout<<testp_goal.toString()<<"Gefunden in Zelle: "<<c_goal_id<<std::endl;//
			}
			else{
				std::cout<<"Nicht gefunden"<<std::endl;
			}

			//Initialisation
			unsigned int c_totalcount=_meshdata->get_cellCount();
			//std::cout<<"Total Number of Cells: "<<c_totalcount<<std::endl;
			bool* closedlist=new bool[c_totalcount];
			bool* inopenlist=new bool[c_totalcount];
			int* predlist=new int[c_totalcount]; // to gain the path from start to goal
			double* costlist=new double[c_totalcount];
			for(unsigned int i=0;i<c_totalcount;i++){
				closedlist[i]=false;
				inopenlist[i]=false;
				predlist[i]=-1;
			}
			std::vector<std::pair< double , MeshCell*> > openlist;
			openlist.push_back(std::make_pair(0.0,start_cell));
			inopenlist[c_start_id]=true;

			MeshCell* act_cell=start_cell;
			int act_id=c_start_id;
			double act_cost=0.0;

			while(act_id!=c_goal_id){
				//astar_print(closedlist,inopenlist,predlist,c_totalcount,act_id);
				if (act_cell==NULL)
					std::cout<<"act_cell=NULL !!"<<std::endl;

				for(unsigned int i=0;i<act_cell->get_edges().size();i++){
					int act_edge_id=act_cell->get_edges().at(i);
					int nb_id=-1;
					// Find neighbouring cell
					if(_meshdata->get_edges().at(act_edge_id)->get_c1()==act_id){
						nb_id=_meshdata->get_edges().at(act_edge_id)->get_c2();
					}
					else if(_meshdata->get_edges().at(act_edge_id)->get_c2()==act_id){
						nb_id=_meshdata->get_edges().at(act_edge_id)->get_c1();
					}
					else{// Error: inconsistant
						Log->Write("Error:\tInconsistant Mesh-Data");
					}
					if (!closedlist[nb_id]){// neighbour-cell not fully evaluated
						MeshCell* nb_cell=_meshdata->getCellAtPos(nb_id);
						double new_cost=act_cost+(act_cell->get_mid()-nb_cell->get_mid()).Norm();
						if(!inopenlist[nb_id]){// neighbour-cell not evaluated at all
							predlist[nb_id]=act_id;
							costlist[nb_id]=new_cost;
							inopenlist[nb_id]=true;

							double f=new_cost+(nb_cell->get_mid()-testp_goal).Norm();
							openlist.push_back(std::make_pair(f,nb_cell));
						}
						else{
							if (new_cost<costlist[nb_id]){
								std::cout<<"ERROR"<<std::endl;
								//found shorter path to nb_cell
								predlist[nb_id]=act_id;
								costlist[nb_id]=new_cost;
								// update nb in openlist
								for(unsigned int j=0;j<openlist.size();j++){
									if(openlist.at(i).second->get_id()==nb_id){
										MeshCell* nb_cell=openlist.at(i).second;
										double f=new_cost+(nb_cell->get_mid()-testp_goal).Norm();
										openlist.at(i)=std::make_pair(f,nb_cell);
										break;
									}
								}
							}
							else{
								// Do nothing: Path is worse
							}
						}
					}
				}

				std::vector<std::pair<double,MeshCell*> >::iterator it=openlist.begin();

				while(it->second->get_id()!=act_id){
					it++;
				}
				closedlist[act_id]=true;
				inopenlist[act_id]=false;
				openlist.erase(it);

				int next_cell_id=-1;
				MeshCell* next_cell=NULL;
				//astar_print(closedlist,inopenlist,predlist,c_totalcount,act_id,openlist); ///////////////
				if (openlist.size()>0){
					//Find cell with best f value
					double min_f=openlist.at(0).first;
					next_cell_id=openlist.at(0).second->get_id();
					//std::cout<<"next_cell_id: "<<next_cell_id<<std::endl;
					next_cell=openlist.at(0).second;
					for(unsigned int j=1;j<openlist.size();j++){
						if (openlist.at(j).first<min_f){
							min_f=openlist.at(j).first;
							next_cell=openlist.at(j).second;
							next_cell_id=openlist.at(j).second->get_id();
						}
					}
					act_id=next_cell_id;
					act_cell=next_cell;
				}
				else{
					Log->Write("Error:\tA* did not find a path");
				}
			}
			//print_path(predlist,c_start_id,c_goal_id);/////////////////
			//astar_print(closedlist,inopenlist,predlist,c_totalcount,act_id,openlist);

			// The next edge on the path
			act_id=c_goal_id;
				while(predlist[act_id]!=c_start_id){
					act_id=predlist[act_id];
				}

				if(c_start_id!=c_goal_id){
					for (unsigned i=0;i<start_cell->get_edges().size();i++){
						 int act_edge_id=start_cell->get_edges().at(i);
						 if(_meshdata->get_edges().at(act_edge_id)->get_c1()==act_id){
							 *edge=_meshdata->get_edges().at(act_edge_id);
							 break;
						}
						 else if(_meshdata->get_edges().at(act_edge_id)->get_c2()==act_id){
							 *edge=_meshdata->get_edges().at(act_edge_id);
							 break;
						 }
						 else{//
						 }
					}
				}
				//std::cout<<(*edge)->toString()<<std::endl;
				if((*edge)!=NULL || c_start_id!=c_goal_id )
					return 0;
				else
					return -1;
}

int MeshRouter::FindExit(Pedestrian* p) {
	//std::cout<<"calling the mesh router"<<std::endl;
	MeshEdge* edge=NULL;
	GetNextEdge(p,&edge);
//	if (edge!=NULL)
//	   std::cout<<edge->toString()<<std::endl;
//	else
//		std::cout<<"Keine Kante gefunden"<<std::endl;

	p->SetExitLine(edge);
	return 0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//return any transition or crossing in the actual room.
	//p->SetExitIndex(-1);
	//p->GetPos();
	//find a proper navigation line
//	p->SetExitLine(_building->GetAllTransitions()[0]);
//	p->SetExitLine(_building->GetTransition(0)GetRoom(p->GetRoomID())->GetSubRoom(p->GetSubRoomID())->GetAllCrossings()[0]);
	//return -1;
}

void MeshRouter::Init(Building* b) {
	_building=b;
	Log->Write("WARNING: \tdo not use this  <<Mesh>>  router !!");

	std::string meshfileName=b->GetFilename()+".nav";
	std::ifstream meshfiled;
	meshfiled.open(meshfileName.c_str(), std::ios::in);
	if(!meshfiled.is_open()){
		Log->Write("ERROR: \tcould not open meshfile <%s>",meshfileName.c_str());
		exit(EXIT_FAILURE);
	}
	std::stringstream meshfile;
	meshfile<<meshfiled.rdbuf();
	meshfiled.close();

	std::vector<Point*> nodes; nodes.clear();
	std::vector<MeshEdge*> edges;
	std::vector<MeshEdge*> outedges;
	std::vector<MeshCellGroup*> mCellGroups;

    unsigned int countNodes=0;
	meshfile>>countNodes;
	for(unsigned int i=0;i<countNodes;i++){
		double temp1,temp2;
		meshfile>>temp1>>temp2;
		nodes.push_back(new Point(temp1,temp2));
	}
	std::cout<<std::setw(2)<<"Read "<<nodes.size()<<" Nodes from file"<<std::endl;


	unsigned int countEdges=0;
	meshfile>>countEdges;
	for(unsigned int i=0;i<countEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		edges.push_back(new MeshEdge(t1,t2,t3,t4,*(nodes.at(t1)),*(nodes.at(t2))));
	}
	std::cout<<"Read "<<edges.size()<<" inner Edges from file"<<std::endl;

	unsigned int countOutEdges=0;
	meshfile>>countOutEdges;
	for(unsigned int i=0;i<countOutEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		outedges.push_back(new MeshEdge(t1,t2,t3,t4,*(nodes.at(t1)),*(nodes.at(t2))));
	}
	std::cout<<"Read "<<outedges.size()<<" outer Edges from file"<<std::endl;

	int tc_id=0;
	while(!meshfile.eof()){
		std::string groupname;
		bool  namefound=false;
		while(!namefound && getline(meshfile,groupname)){
			if (groupname.size()>2){
				namefound=true;
			}
		}
		if (!meshfile.eof()){

			unsigned int countCells=0;
			meshfile>>countCells;

			std::vector<MeshCell*> mCells; mCells.clear();
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
				//double* normvec=new double[3];
				double normvec[3];
				for (unsigned int j=0;j<3;j++){
					double tmp=0.0;
					meshfile>>tmp;
					normvec[j]=tmp;
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
				mCells.push_back(new MeshCell(midx,midy,node_id,normvec,edge_id,wall_id,tc_id));
				tc_id++;
			}
			mCellGroups.push_back(new MeshCellGroup(groupname,mCells));
		}
	}

	_meshdata=new MeshData(nodes,edges,outedges,mCellGroups);
}



