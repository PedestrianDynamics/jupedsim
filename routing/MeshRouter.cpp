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
	unsigned int c_totalcount,int act_id,vector<pair<double,MeshCell*> >openlist){
	cout<<"----------------------------------"<<endl;
	cout<<"act_id: "<<act_id<<endl;
	cout<<"Closed-List"<<endl;
	for(unsigned int i=0;i<c_totalcount;i++)
		cout<<(closedlist[i]?"1 ":"0 ");
	cout<<endl;
	cout<<"Inopen-List"<<endl;
		for(unsigned int i=0;i<c_totalcount;i++)
			cout<<(inopenlist[i]?"1 ":"0 ");
		cout<<endl;
	cout<<"Predecessor-List"<<endl;
	for(unsigned int i=0;i<c_totalcount;i++)
		(predlist[i]!=-1?cout<<predlist[i]<<" ":cout<<"* ");
	cout<<endl;
	cout<<"Openlist"<<endl;
	for(unsigned int i=0;i<openlist.size();i++)
		cout<<openlist.at(i).second->Get_id()<<"(f="<<openlist.at(i).first<<") ";
	cout<<endl;
	cout<<"----------------------------------"<<endl;
}

void print_path(int* predlist,int c_start_id,int c_goal_id){
	cout<<"reverse path:"<<endl;
	int act_id=c_goal_id;
	while(act_id!=c_start_id){
		cout<<act_id<<" "<<endl;
		act_id=predlist[act_id];
	}
	cout<<c_start_id<<endl;
}


/* Helper for Funnel
     \   2 /
       \   /
    3   X apex  1
      /    \
     /   0  \
 right   left
 */
int TestinFunnel(Point apex, Point left,Point right,Point test){

	double n1x=right.GetX();
	double n1y=right.GetX();
	double n2x= apex.GetX();
	double n2y= apex.GetY();
	Point temp_nxny(n2y-n1y,n1x-n2x);
	Point temp_xy=test-Point(n1x,n1y);

	bool r_test;
	if (temp_xy.ScalarP(temp_nxny)>0)
		r_test=true;
	else
		r_test=false;

	n1x=n2x;
	n1y=n2y;
	n2x=left.GetX();
	n2y=left.GetY();
	temp_nxny=Point(n2y-n1y,n1x-n2x);
	temp_xy=test-Point(n1x,n1y);

	bool l_test;
	if (temp_xy.ScalarP(temp_nxny)>0)
	   l_test=true;
	else
		l_test=false;

	if (r_test){
		if(l_test)
		   return 0;
		else
			return 1;
	}else{
		if(l_test)
		   return 3;
		else
			return 2;
	}
}

NavLine MeshRouter::Funnel(Point& start,Point& goal,vector<MeshEdge*> edge_path){

	if(edge_path.empty()){
		// Start and End Point in same Cell
		Line goal_line(goal,goal);
		return NavLine(goal_line);

	}
	else{
		int goal_cell_id=-1;
		MeshCell* goal_cell=_meshdata->FindCell(goal,goal_cell_id);

		Point apex=start;
		int act_cell_id=-1;
		int loc_ind=-1; // local index of first node to be found in startphase
		unsigned int path_ind=0;
		int loc_ind_left,loc_ind_right; // local Indices of nodes creating the wedge
		int ind_left,ind_right;// Indices of nodes creating the wedge
		int ind_run_left,ind_run_right; //Indices of nodes creating the funnel
		Point point_left,point_right; // Nodes creatin the wedge
		MeshCell* start_cell=_meshdata->FindCell(apex,act_cell_id);
		for (unsigned int i=0;i<start_cell->Get_nodes().size();i++){
			if (start_cell->Get_nodes().at(i)==edge_path.at(0)->Get_n1())
				loc_ind=i;
		}
		unsigned int nodes_of_cell=start_cell->Get_nodes().size();
		if (start_cell->Get_nodes().at((loc_ind+1)%nodes_of_cell)==edge_path.at(0)->Get_n2()){
			loc_ind_left=loc_ind;
			loc_ind_right=edge_path.at(0)->Get_n2();
		}
		else if(start_cell->Get_nodes().at((loc_ind-1)%nodes_of_cell)==edge_path.at(0)->Get_n2()){
			loc_ind_left=loc_ind;
			loc_ind_right=edge_path.at(0)->Get_n2();
		}
		else{
			cout<<"Error: Path not consistant with cell"<<endl;
			loc_ind_left=-1;
			loc_ind_right=-1;
		}

		ind_left=start_cell->Get_nodes().at(loc_ind_left);
		ind_right=start_cell->Get_nodes().at(loc_ind_right);

		point_left=*_meshdata->Get_nodes().at(ind_left);
		point_right=*_meshdata->Get_nodes().at(ind_right);
		ind_run_left=ind_left;
		ind_run_right=ind_right;


		Point point_run_left=point_left;
		Point point_run_right=point_right;

		bool apex_found=false;
		// lengthen the funnel at side
		bool run_left=true,run_right=true;
		while(!apex_found){
			if(path_ind<edge_path.size()){ // Last Cell not yet reached =>Continue or node on edge

				//  Run left(right) till next MeshEdge to be traversed
				while(run_left){
					//...
				}
				while(run_right){
					//..
				}
				// Test for new Points to be in the wedge of start
				int test_l=TestinFunnel(start,point_left,point_right,point_run_left);
				int test_r=TestinFunnel(start,point_left,point_right,point_run_right);

				if(test_l==0 && test_r==0){ //Narrow wedge

				}
				else if(test_l==1 && test_r==1){// apex=left

					apex_found=true;
				}
				else if(test_l==3 && test_r==3){//apex=right

					apex_found=true;
				}
				else if(test_l==1 && test_r==3){ //  Widen wedge

				}
				else{// Corrupted data

				}

				path_ind++;
			}
			else{ // goal in actual cell => apex= goal or node on edge

				apex_found=true;
			}

		}
	}
	return NavLine();
}

int MeshRouter::AStar(Pedestrian* p,MeshEdge** edge){
	cout<<"A*"<<endl;
				int c_start_id;
				Point  testp_start=p->GetPos();
				MeshCell* start_cell=_meshdata->FindCell(testp_start,c_start_id);
				if(start_cell!=NULL){
				}
				else{
					cout<<"Nicht gefunden"<<endl;
				}
				int c_goal_id;
				Point testp_goal(-6,17);
				MeshCell* goal_cell=_meshdata->FindCell(testp_goal,c_goal_id);
				if(goal_cell!=NULL){
					//cout<<testp_goal.toString()<<"Gefunden in Zelle: "<<c_goal_id<<endl;//
				}
				else{
					cout<<"Nicht gefunden"<<endl;
				}

				//Initialisation
				unsigned int c_totalcount=_meshdata->Get_cellCount();
				bool* closedlist=new bool[c_totalcount];
				bool* inopenlist=new bool[c_totalcount];
				int* predlist=new int[c_totalcount]; // to gain the path from start to goal (contains cell ID)
				MeshEdge** predEdgelist= new MeshEdge*[c_totalcount];
				double* costlist=new double[c_totalcount];
				for(unsigned int i=0;i<c_totalcount;i++){
					closedlist[i]=false;
					inopenlist[i]=false;
					predlist[i]=-1;
					predEdgelist[i]=NULL;
				}
				vector<pair< double , MeshCell*> > openlist;
				openlist.push_back(make_pair(0.0,start_cell));
				inopenlist[c_start_id]=true;

				MeshCell* act_cell=start_cell;
				int act_id=c_start_id;
				double act_cost=0.0;
				//cout<<"vor while"<<endl;
				while(act_id!=c_goal_id){
					if (act_cell==NULL)
						cout<<"act_cell=NULL !!"<<endl;

					for(unsigned int i=0;i<act_cell->Get_edges().size();i++){
						int act_edge_id=act_cell->Get_edges().at(i);
						int nb_id=-1;
						// Find neighbouring cell
						if(_meshdata->Get_edges().at(act_edge_id)->Get_c1()==act_id){
							nb_id=_meshdata->Get_edges().at(act_edge_id)->Get_c2();
						}
						else if(_meshdata->Get_edges().at(act_edge_id)->Get_c2()==act_id){
							nb_id=_meshdata->Get_edges().at(act_edge_id)->Get_c1();
						}
						else{// Error: inconsistant
							Log->Write("Error:\tInconsistant Mesh-Data");
						}
						if (!closedlist[nb_id]){// neighbour-cell not fully evaluated
							MeshCell* nb_cell=_meshdata->GetCellAtPos(nb_id);
							double new_cost=act_cost+(act_cell->Get_mid()-nb_cell->Get_mid()).Norm();
							if(!inopenlist[nb_id]){// neighbour-cell not evaluated at all
								predlist[nb_id]=act_id;
								predEdgelist[nb_id]=_meshdata->Get_edges().at(act_edge_id);
								costlist[nb_id]=new_cost;
								inopenlist[nb_id]=true;

								double f=new_cost+(nb_cell->Get_mid()-testp_goal).Norm();
								openlist.push_back(make_pair(f,nb_cell));
							}
							else{
								if (new_cost<costlist[nb_id]){
									cout<<"ERROR"<<endl;
									//found shorter path to nb_cell
									predlist[nb_id]=act_id;
									predEdgelist[nb_id]=_meshdata->Get_edges().at(act_edge_id);
									costlist[nb_id]=new_cost;
									// update nb in openlist
									for(unsigned int j=0;j<openlist.size();j++){
										if(openlist.at(i).second->Get_id()==nb_id){
											MeshCell* nb_cell=openlist.at(i).second;
											double f=new_cost+(nb_cell->Get_mid()-testp_goal).Norm();
											openlist.at(i)=make_pair(f,nb_cell);
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

					vector<pair<double,MeshCell*> >::iterator it=openlist.begin();

					while(it->second->Get_id()!=act_id){
						it++;
					}
					closedlist[act_id]=true;
					inopenlist[act_id]=false;
					openlist.erase(it);

					int next_cell_id=-1;
					MeshCell* next_cell=NULL;
					if (openlist.size()>0){
						//Find cell with best f value
						double min_f=openlist.at(0).first;
						next_cell_id=openlist.at(0).second->Get_id();
						//cout<<"next_cell_id: "<<next_cell_id<<endl;
						next_cell=openlist.at(0).second;
						for(unsigned int j=1;j<openlist.size();j++){
							if (openlist.at(j).first<min_f){
								min_f=openlist.at(j).first;
								next_cell=openlist.at(j).second;
								next_cell_id=openlist.at(j).second->Get_id();
							}
						}
						act_id=next_cell_id;
						act_cell=next_cell;
					}
					else{
						Log->Write("Error:\tA* did not find a path");
					}
				}
				delete[] closedlist;
				delete[] inopenlist;
				delete[] costlist;
				//cout<<"erstelle Pfad"<<endl;
				vector <MeshEdge*> path_edges;

				// The next edge on the path + building the reverse Path through edges
				act_id=c_goal_id;
					while(predlist[act_id]!=c_start_id){
						path_edges.push_back(predEdgelist[act_id]);
						act_id=predlist[act_id];
					}
					// Reverse the reversed path to gain path from start to goal
					reverse(path_edges.begin(),path_edges.end());
					delete[] predlist;

					*edge=&**path_edges.begin();
					delete[] predEdgelist;

					return 0;
					//return path_edges;
}


int MeshRouter::GetNextEdge(Pedestrian* p, MeshEdge** edge){
	/*
			 * A* TEST IMPLEMENTATION
			 */

			int c_start_id;
			//Point testp_start(0,0);
			Point  testp_start=p->GetPos();
			//cout<<testp_start.toString()<<endl;;
			MeshCell* start_cell=_meshdata->FindCell(testp_start,c_start_id);
			if(start_cell!=NULL){
				//cout<<testp_start.toString()<<"Gefunden in Zelle: "<<c_start_id<<endl;
			}
			else{
				cout<<"Nicht gefunden"<<endl;
			}
			int c_goal_id;
			Point testp_goal(-6,17);
			MeshCell* goal_cell=_meshdata->FindCell(testp_goal,c_goal_id);
			if(goal_cell!=NULL){
				//cout<<testp_goal.toString()<<"Gefunden in Zelle: "<<c_goal_id<<endl;//
			}
			else{
				cout<<"Nicht gefunden"<<endl;
			}

			//Initialisation
			unsigned int c_totalcount=_meshdata->Get_cellCount();
			//cout<<"Total Number of Cells: "<<c_totalcount<<endl;
			bool* closedlist=new bool[c_totalcount];
			bool* inopenlist=new bool[c_totalcount];
			int* predlist=new int[c_totalcount]; // to gain the path from start to goal
			double* costlist=new double[c_totalcount];
			for(unsigned int i=0;i<c_totalcount;i++){
				closedlist[i]=false;
				inopenlist[i]=false;
				predlist[i]=-1;
			}


			MeshCell* act_cell=start_cell;
			int act_id=c_start_id;
			double f= (act_cell->Get_mid()-testp_goal).Norm();/////////////////////////////////////////////////////////////////////////
			double act_cost=f;//
			vector<pair< double , MeshCell*> > openlist;//
			openlist.push_back(make_pair(f,start_cell));//
			costlist[act_id]=f;
			inopenlist[c_start_id]=true;////////////////////////////////////////////////////////////////////////////////////////////////////////

			while(act_id!=c_goal_id){
				act_cost=costlist[act_id]; ////////////////////////////////////////////////////////////////////////////////////////////////////////
				//astar_print(closedlist,inopenlist,predlist,c_totalcount,act_id);
				if (act_cell==NULL)
					cout<<"act_cell=NULL !!"<<endl;

				for(unsigned int i=0;i<act_cell->Get_edges().size();i++){
					int act_edge_id=act_cell->Get_edges().at(i);
					int nb_id=-1;
					// Find neighbouring cell
					if(_meshdata->Get_edges().at(act_edge_id)->Get_c1()==act_id){
						nb_id=_meshdata->Get_edges().at(act_edge_id)->Get_c2();
					}
					else if(_meshdata->Get_edges().at(act_edge_id)->Get_c2()==act_id){
						nb_id=_meshdata->Get_edges().at(act_edge_id)->Get_c1();
					}
					else{// Error: inconsistant
						Log->Write("Error:\tInconsistant Mesh-Data");
					}
					if (!closedlist[nb_id]){// neighbour-cell not fully evaluated
						MeshCell* nb_cell=_meshdata->GetCellAtPos(nb_id);
						double new_cost=act_cost+(act_cell->Get_mid()-nb_cell->Get_mid()).Norm();
						if(!inopenlist[nb_id]){// neighbour-cell not evaluated at all
							predlist[nb_id]=act_id;
							costlist[nb_id]=new_cost;
							inopenlist[nb_id]=true;

							double f=new_cost+(nb_cell->Get_mid()-testp_goal).Norm();
							openlist.push_back(make_pair(f,nb_cell));
						}
						else{
							if (new_cost<costlist[nb_id]){
								cout<<"ERROR"<<endl;
								//found shorter path to nb_cell
								predlist[nb_id]=act_id;
								costlist[nb_id]=new_cost;
								// update nb in openlist
								for(unsigned int j=0;j<openlist.size();j++){
									if(openlist.at(i).second->Get_id()==nb_id){
										MeshCell* nb_cell=openlist.at(i).second;
										double f=new_cost+(nb_cell->Get_mid()-testp_goal).Norm();
										openlist.at(i)=make_pair(f,nb_cell);
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

				vector<pair<double,MeshCell*> >::iterator it=openlist.begin();

				while(it->second->Get_id()!=act_id){
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
					next_cell_id=openlist.at(0).second->Get_id();
					//cout<<"next_cell_id: "<<next_cell_id<<endl;
					next_cell=openlist.at(0).second;
					for(unsigned int j=1;j<openlist.size();j++){
						if (openlist.at(j).first<min_f){
							min_f=openlist.at(j).first;
							next_cell=openlist.at(j).second;
							next_cell_id=openlist.at(j).second->Get_id();
						}
					}
					act_id=next_cell_id;
					act_cell=next_cell;
				}
				else{
					Log->Write("Error:\tA* did not find a path");
				}
			}
			delete[] closedlist;
			delete[] inopenlist;
			delete[] costlist;
			//print_path(predlist,c_start_id,c_goal_id);/////////////////
			//astar_print(closedlist,inopenlist,predlist,c_totalcount,act_id,openlist);

			// The next edge on the path
			act_id=c_goal_id;
				while(predlist[act_id]!=c_start_id){
					act_id=predlist[act_id];
				}

				if(c_start_id!=c_goal_id){
					for (unsigned i=0;i<start_cell->Get_edges().size();i++){
						 int act_edge_id=start_cell->Get_edges().at(i);
						 if(_meshdata->Get_edges().at(act_edge_id)->Get_c1()==act_id){
							 *edge=_meshdata->Get_edges().at(act_edge_id);
							 break;
						}
						 else if(_meshdata->Get_edges().at(act_edge_id)->Get_c2()==act_id){
							 *edge=_meshdata->Get_edges().at(act_edge_id);
							 break;
						 }
						 else{//
						 }
					}
				}
				delete[] predlist;

				//MeshEdge* etst=*(AStar(p).begin());
				//*edge=*(AStar(p).begin());

				//cout<<(*edge)->toString()<<endl;
				if((*edge)!=NULL || c_start_id!=c_goal_id )
					return 0;
				else
					return -1;
}

int MeshRouter::FindExit(Pedestrian* p) {
	//cout<<"calling the mesh router"<<endl;
	Point  testp_start=p->GetPos();
	int c_start_id=-1;
	_meshdata->FindCell(testp_start,c_start_id);

	MeshEdge* edge=NULL;
	MeshEdge* edge2=NULL;
	NavLine* nextline=NULL;
	if(p->GetCellPos()==c_start_id){
	//if(false){
		nextline=p->GetExitLine();
	}else{
		//GetNextEdge(p,&edge);
		//cout<<"vor  A*"<<endl;
		//vector<MeshEdge*> astar_path=AStar(p);
		//AStar(p,&edge2);

		//if (astar_path.empty())
		if(true){
			GetNextEdge(p,&edge);
			AStar(p,&edge2);
			//edge2=*(astar_path.begin());
			if(*edge!=*edge2)
			cout<<"dif"<<endl;
		}
		else
			AStar(p,&edge);
			//edge=*(astar_path.begin());
		//cout<<"nach A*"<<endl;
		nextline=dynamic_cast<NavLine*>(edge);
		if(nextline==NULL)
			cout<<"1. Nextline=NULL!"<<endl;
	}

	if(nextline==NULL)
				cout<<"2. Nextline=NULL!"<<endl;

	p->SetExitLine(nextline);
	p->SetCellPos(c_start_id);
	return 0;

	//return any transition or crossing in the actual room.
	//p->SetExitIndex(-1);
}

void MeshRouter::Init(Building* b) {
	_building=b;
	Log->Write("WARNING: \tdo not use this  <<Mesh>>  router !!");

	string meshfileName=b->GetFilename()+".nav";
	ifstream meshfiled;
	meshfiled.open(meshfileName.c_str(), ios::in);
	if(!meshfiled.is_open()){
		Log->Write("ERROR: \tcould not open meshfile <%s>",meshfileName.c_str());
		exit(EXIT_FAILURE);
	}
	stringstream meshfile;
	meshfile<<meshfiled.rdbuf();
	meshfiled.close();

	vector<Point*> nodes; //nodes.clear();
	vector<MeshEdge*> edges;
	vector<MeshEdge*> outedges;
	vector<MeshCellGroup*> mCellGroups;

    unsigned int countNodes=0;
	meshfile>>countNodes;
	for(unsigned int i=0;i<countNodes;i++){
		double temp1,temp2;
		meshfile>>temp1>>temp2;
		nodes.push_back(new Point(temp1,temp2));
	}
	cout<<setw(2)<<"Read "<<nodes.size()<<" Nodes from file"<<endl;


	unsigned int countEdges=0;
	meshfile>>countEdges;
	for(unsigned int i=0;i<countEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		edges.push_back(new MeshEdge(t1,t2,t3,t4,*(nodes.at(t1)),*(nodes.at(t2))));
	}
	cout<<"Read "<<edges.size()<<" inner Edges from file"<<endl;

	unsigned int countOutEdges=0;
	meshfile>>countOutEdges;
	for(unsigned int i=0;i<countOutEdges;i++){
		int t1,t2,t3,t4;
		meshfile>>t1>>t2>>t3>>t4;
		outedges.push_back(new MeshEdge(t1,t2,t3,t4,*(nodes.at(t1)),*(nodes.at(t2))));
	}
	cout<<"Read "<<outedges.size()<<" outer Edges from file"<<endl;

	int tc_id=0;
	while(!meshfile.eof()){
		string groupname;
		bool  namefound=false;
		while(!namefound && getline(meshfile,groupname)){
			if (groupname.size()>2){
				namefound=true;
			}
		}
		if (!meshfile.eof()){

			unsigned int countCells=0;
			meshfile>>countCells;

			vector<MeshCell*> mCells; mCells.clear();
			for(unsigned int i=0;i<countCells;i++){
				double midx,midy;
				meshfile>>midx>>midy;
				unsigned int countNodes=0;
				meshfile>>countNodes;
				vector<int> node_id;
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
				vector<int> edge_id;
				for(unsigned int j=0;j<countEdges;j++){
					int tmp;
					meshfile>>tmp;
					edge_id.push_back(tmp);
				}
				unsigned int countWalls=0;
				meshfile>>countWalls;
				vector<int> wall_id;
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

	/*
	 *  Test
	 */
	//Point test_l(3,0), test_r(0,0);
	//Point test_apex(2,3),test_test(2,-2);
	//cout<<"Test funnel "<<TestinFunnel(test_apex,test_l,test_r,test_test)<<endl;
}



