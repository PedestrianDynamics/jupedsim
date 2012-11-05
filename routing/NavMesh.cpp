/*
 * NavMesh.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ulrich Kemloh
 */

#include "NavMesh.h"

#define _DEBUG


NavMesh::NavMesh() {
	pBuilding=NULL;
}

NavMesh::~NavMesh() {
}

void NavMesh::BuildNavMesh(Building* b) {

	pBuilding=b;

	std::map<int,int> subroom_to_node;


	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* r = pBuilding->GetRoom(i);
		string caption = r->GetCaption();

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);

			//vertices
			const vector<Point>& pol = s->GetPolygon();

			// Vertices
			for (unsigned int p = 0; p < pol.size(); p++) {
				Vertex* v = new Vertex();
				v->pPos= pol[p];
				AddVertex(v);
			}

			//Nodes
			Node* node = new Node();
			node->pGroup = r->GetCaption();
			node->pCentroid = s->GetCentroid();
			node->pNormalVec[0]=0;
			node->pNormalVec[1]=0;
			node->pNormalVec[2]=0;
			for (unsigned int p = 0; p < pol.size(); p++) {
				node->pHull.push_back(*(GetVertex(pol[p])));
			}


			//edges are transitions and crossings
			const vector<Crossing*>& crossings = s->GetAllCrossings();
			for (unsigned c = 0; c < crossings.size(); c++) {

				Edge* e= new Edge();
				int node0 = crossings[c]->GetSubRoom1()->GetUID();
				int node1 = crossings[c]->GetSubRoom2()->GetUID();
				Point centroid0=crossings[c]->GetSubRoom1()->GetCentroid();
				Point centroid1=crossings[c]->GetSubRoom2()->GetCentroid();

				if (node0 > node1) {
					swap(node0, node1);
					swap(centroid0,centroid1);
				}

				e->pNode0=node0;
				e->pNode1=node1;

				//first attempt
				Point P0 = crossings[c]->GetPoint1();
				Point P1 = crossings[c]->GetPoint2();
				Point D0 = P1 - P0;
				if (D0.Det(centroid0) < 0) {
					e->pDisp=D0;
					e->pStart= *GetVertex(P0);

				}else{
					e->pStart= *GetVertex(P1);
					e->pDisp=Point(0,0)-D0;
				}
				AddEdge(e);
			}


			const vector<Transition*>& transitions = s->GetAllTransitions();
			for (unsigned t = 0; t < transitions.size(); t++) {
				int node0 = transitions[t]->GetSubRoom1()->GetUID();
				int node1 =
						(transitions[t]->GetSubRoom2() == NULL) ?
								-1 : transitions[t]->GetSubRoom2()->GetUID();

				Point centroid0 = transitions[t]->GetSubRoom1()->GetCentroid();

				Point centroid1 =
						(transitions[t]->GetSubRoom2() == NULL) ?
								transitions[t]->GetCentre() :
								transitions[t]->GetSubRoom2()->GetCentroid();


				Edge* e= new Edge();

				if (node0 > node1) {
					swap(node0, node1);
					swap(centroid0,centroid1);
				}

				e->pNode0=node0;
				e->pNode1=node1;

				//first attempt
				Point P0 = transitions[t]->GetPoint1();
				Point P1 = transitions[t]->GetPoint2();
				Point D0 = P1 - P0;
				if (D0.Det(centroid0) < 0) {
					e->pDisp=D0;
					e->pStart= *GetVertex(P0);

				}else{
					e->pStart= *GetVertex(P1);
					e->pDisp=Point(0,0)-D0;
				}

				AddEdge(e);
			}



			//determine the group based on the crossings
			if(crossings.size()==1 && transitions.size()==0){
				node->pGroup="seat";
			}else {
				if(crossings.size()==2){
					if(crossings[0]->Length()==crossings[1]->Length())
						node->pGroup="seat";
				}
			}

			//obstacles
			const vector<Wall>& walls = s->GetAllWalls();
			for (unsigned w = 0; w < walls.size(); w++) {

				const Point& centroid0 = s->GetCentroid();
				int node0 = s->GetUID();

				Obst* o= new Obst();
				o->pNode0=node0;
				o->pNextObst=-1;

				//first attempt
				Point P0 = walls[w].GetPoint1();
				Point P1 = walls[w].GetPoint2();
				Point D0 = P1 - P0;
				if (D0.Det(centroid0) < 0) {
					o->pDisp=D0;
					o->pStart= *GetVertex(P0);

				}else{
					o->pStart= *GetVertex(P1);
					o->pDisp=Point(0,0)-D0;
				}

				AddObst(o);
				node->pObstacles.push_back(o->id);
			}
			AddNode(node);
			//			subroom_to_node[node->id]=s->GetUID();
			subroom_to_node[s->GetUID()]=node->id;
		}
	}

	//for(std::map<int, int>::iterator p = subroom_to_node.begin(); p != subroom_to_node.end(); ++p) {
	//	cout<<" [ "<<p->first<<", " << p->second<<" m ]";
	//}

	//	for(int i=0;i<pNodes.size();i++){
	//		cout<<i<<" : "<<pNodes[i]->id<<" : "<<pNodes[i]->pGroup<<endl;
	//	}

	std::sort(pNodes.begin(), pNodes.end(),Node());

	//	doing the mapping
	for(unsigned int i=0;i<pNodes.size();i++){
		subroom_to_node[pNodes[i]->id]=i;
		pNodes[i]->id=i;
	}


	//normalizing the IDs
	for (unsigned int e=0;e<pEdges.size();e++){
		pEdges[e]->pNode0=subroom_to_node[pEdges[e]->pNode0];
		pEdges[e]->pNode1=subroom_to_node[pEdges[e]->pNode1];
	}

	for (unsigned int ob=0;ob<pObst.size();ob++){
		pObst[ob]->pNode0=subroom_to_node[pObst[ob]->pNode0];
	}


}

void NavMesh::WriteToFileTraVisTo(std::string fileName) {
	ofstream file(fileName.c_str());
	file.precision(2);
	file<<fixed;

	if(file.is_open()==false){
		cout <<"could not open the file: "<<fileName<<endl;
		return;
	}

	//writing the header
	file<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl
			<<"<trajectoriesDataset>"<<endl
			<<"\t<header formatVersion = \"1.0\">"<<endl
			<<"\t\t<agents>3</agents>"<<endl
			<<"\t\t<seed>0</seed>"<<endl
			<<"\t\t<frameRate>10</frameRate>"<<endl
			<<"\t</header>"<<endl
			<<endl
			<<endl
			<<"\t<geometry>"<<endl;

	//writing the nodes
	vector<int> nodes_to_plot;
	//nodes_to_plot.push_back(1);


	for (unsigned int n=0;n<pNodes.size();n++){

		Node* node=pNodes[n];
		int node_id=node->id;
		if(nodes_to_plot.size()!=0)
			if (IsElementInVector(nodes_to_plot, node_id) == false)
				continue;

		file<<"\t\t<label centerX=\""<<node->pCentroid.GetX()*FAKTOR<<"\" centerY=\""<<node->pCentroid.GetY()*FAKTOR<<"\" centerZ=\"0\" text=\""<<node->id <<"\" color=\"100\" />"<<endl;

		for(unsigned int i=0;i<node->pHull.size();i++){
			double x=pVertices[node->pHull[i].id]->pPos.GetX()*FAKTOR;
			double y=pVertices[node->pHull[i].id]->pPos.GetY()*FAKTOR;
			file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
			file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<node->pHull[i].id<<"\" color=\"20\" />"<<endl;
		}

		file<<endl;

		for(unsigned int i=0;i<node->pObstacles.size();i++){

			for(unsigned int i=0;i<node->pObstacles.size();i++){
				double x1=pObst[node->pObstacles[i]]->pStart.pPos.GetX()*FAKTOR;
				double y1=pObst[node->pObstacles[i]]->pStart.pPos.GetY()*FAKTOR;
				double x2=pObst[node->pObstacles[i]]->pDisp.GetX()*FAKTOR +x1;
				double y2=pObst[node->pObstacles[i]]->pDisp.GetY()*FAKTOR +y1;

				file<<"\t\t<wall>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
				file<<"\t\t</wall>"<<endl;
			}

		}
		file<<endl;

		for(unsigned int i=0;i<pEdges.size();i++){
			Edge* edge=pEdges[i];

			if(edge->pNode0==node_id || edge->pNode1==node_id ){
				double x1=edge->pStart.pPos.GetX()*FAKTOR;
				double y1=edge->pStart.pPos.GetY()*FAKTOR;
				double x2=edge->pDisp.GetX()*FAKTOR +x1;
				double y2=edge->pDisp.GetY()*FAKTOR +y1;

				file<<"\t\t<door>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
				file<<"\t\t</door>"<<endl;
			}

		}
	}

	file<<"\t</geometry>"<<endl;
	file.close();
}
void NavMesh::WriteToFile(std::string fileName) {

	ofstream file(fileName.c_str());
	file.precision(2);
	file<<fixed;

	if(file.is_open()==false){
		cout <<"could not open the file: "<<fileName<<endl;
		return;
	}

	//write the vertices
	file<<"# vertices section"<<endl;
	file<<pVertices.size()<<endl;
	for (unsigned int v=0;v<pVertices.size();v++){
		file<<"\t"<<pVertices[v]->pPos.GetX()<<" " <<pVertices[v]->pPos.GetY()<<endl;

	}

	//write the edges
	file<<endl<<"# edges section"<<endl;
	file<<pEdges.size()<<endl;
	for (unsigned int e=0;e<pEdges.size();e++){
		file<<pEdges[e]->pStart.pPos.GetX()<<" " <<pEdges[e]->pStart.pPos.GetY()<<endl;
		file<<"\t"<<pEdges[e]->pDisp.GetX()<<" " <<pEdges[e]->pDisp.GetY()<<endl;
		file<<"\t"<<pEdges[e]->pNode0<<" " <<pEdges[e]->pNode1<<endl;
	}


	//write the obstacles
	file<<endl<<"# Obstacles section"<<endl;
	file<<pObst.size()<<endl;
	for (unsigned int ob=0;ob<pObst.size();ob++){
		file<<pObst[ob]->pStart.pPos.GetX()<<" " <<pObst[ob]->pStart.pPos.GetY()<<endl;
		file<<"\t"<<pObst[ob]->pDisp.GetX()<<" " <<pObst[ob]->pDisp.GetY()<<endl;
		file<<"\t"<<pObst[ob]->pNode0<<endl;
		file<<"\t"<<pObst[ob]->pNextObst<<endl;
	}

	//write the nodes
	file<<endl<<"# Nodes section"<<endl;

	std::map<string,int> ngroup_to_size;
	for (unsigned int n=0;n<pNodes.size();n++){
		ngroup_to_size[pNodes[n]->pGroup]++;
	}

	string previousGroup= pNodes[0]->pGroup;
	file<<previousGroup<<endl;
	file<<ngroup_to_size[previousGroup]<<endl;

	for (unsigned int n=0;n<pNodes.size();n++){
		Node* node=pNodes[n];
		string actualGroup=node->pGroup;
		if(actualGroup!=previousGroup){
			previousGroup=actualGroup;
			file<<"# Node group"<<endl;
			file<<previousGroup<<endl;
			file<<ngroup_to_size[previousGroup]<<endl;
		}

		file<<node->id<<endl;
		file<<node->pCentroid.GetX()<<" "<<node->pCentroid.GetY()<<endl;
		file<<"\t"<<node->pHull.size()<<" ";
		for(unsigned int i=0;i<node->pHull.size();i++){
			file<<node->pHull[i].id<<" ";
		}
		file<<endl;

		file<<"\t"<<node->pObstacles.size()<<" ";
		for(unsigned int i=0;i<node->pObstacles.size();i++){
			file<<node->pObstacles[i]<<" ";
		}
		file<<endl;
		file<<"\t"<<node->pNormalVec[0]<<" "<<node->pNormalVec[1]<<" "<<node->pNormalVec[2]<<endl;
	}

	file.close();
}

void NavMesh::AddVertex(Vertex* v) {
	for (unsigned int vc = 0; vc < pVertices.size(); vc++) {
		if (pVertices[vc]->pPos.operator ==(v->pPos)) {
#ifdef _DEBUG
			cout << "vertex already present:" << pVertices[vc]->id << endl;
#endif
			return;
		}
	}
	if (pVertices.size() == 0) {
		v->id = 0;
	} else {
		v->id = pVertices[pVertices.size() - 1]->id + 1;
	}
	pVertices.push_back(v);
}

void NavMesh::AddEdge(Edge* e) {
	if (IsElementInVector(pEdges, e) == false) {
		if (pEdges.size() == 0) {
			e->id = 0;
		} else {
			e->id = pEdges[pEdges.size() - 1]->id + 1;
		}
		pEdges.push_back(e);
	} else {
#ifdef _DEBUG
		cout << "Edge already present:" << e->id << endl;
#endif
	}
}

void NavMesh::AddObst(Obst* o) {
	if (IsElementInVector(pObst, o) == false) {
		if (pObst.size() == 0) {
			o->id = 0;
		} else {
			o->id = pObst[pObst.size() - 1]->id + 1;
		}
		pObst.push_back(o);
	} else {
#ifdef _DEBUG
		cout << "Obstacles already present:" << o->id << endl;
#endif
	}
}

void NavMesh::AddNode(Node* n) {
	if (IsElementInVector(pNodes, n) == false) {
		if (pNodes.size() == 0) {
			n->id = 0;
		} else {
			n->id = pNodes[pNodes.size() - 1]->id + 1;
		}
		pNodes.push_back(n);
	} else {
#ifdef _DEBUG
		cout << "Node already present:" << n->id << endl;
#endif
	}
}



NavMesh::Vertex* NavMesh::GetVertex(const Point& p) {

	for(unsigned int v=0;v<pVertices.size();v++){
		if(pVertices[v]->pPos.operator ==(p)){
			return pVertices[v];
		}
	}
#ifdef _DEBUG
	cout<<"Vertex not found: "<< p.GetX()<<":"<<p.GetY()<<endl;
	cout<<"Adding "<<endl;
	cout<<"pVertices.size()="<<pVertices.size()<<endl;
#endif
	Vertex* v = new Vertex();
	v->pPos= p;
	AddVertex(v);
	//this could lead to an infinite loop if the point cannot be added and cannot be found
	return GetVertex(p);

	//exit(EXIT_FAILURE);
}
