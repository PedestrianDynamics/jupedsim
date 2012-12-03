/*
 * NavMesh.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ulrich Kemloh
 */

#include "NavMesh.h"
#include "../MCD/GeomPoly.h"
#include "../MCD/GeomVector.h"
//#include "../MCD/AlgorithmMCD.h"
#include "DTriangulation.h"

//#define _DEBUG 1



NavMesh::NavMesh(Building* b) {
	pBuilding=b;
}

NavMesh::~NavMesh() {
}

void NavMesh::BuildNavMesh() {

	//	test_triangulation();exit(0);
	std::map<int,int> subroom_to_node;

	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* r = pBuilding->GetRoom(i);
		string caption = r->GetCaption();

		//skip the virtual room containing the complete geometry
		//if(r->GetCaption()=="outside") continue;

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);

			//vertices
			const vector<Point>& pol = s->GetPolygon();

			// Vertices
			for (unsigned int p = 0; p < pol.size(); p++) {
				JVertex* v = new JVertex();
				v->pPos= pol[p];
				if(AddVertex(v)==-1) {
					delete v;
				}
			}

			//Nodes vertices
			JNode* node = new JNode();
			node->pGroup = r->GetCaption();
			node->pCentroid = s->GetCentroid();
			node->pNormalVec[0]=0;
			node->pNormalVec[1]=0;
			node->pNormalVec[2]=r->GetZPos();

			for (unsigned int p = 0; p < pol.size(); p++) {
				node->pHull.push_back(*(GetVertex(pol[p])));
			}


			//edges are transitions and crossings
			const vector<Crossing*>& crossings = s->GetAllCrossings();
			for (unsigned c = 0; c < crossings.size(); c++) {

				JEdge* e= new JEdge();
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
				Point D1 = centroid0-P0;
				if (D0.Det(D1) < 0) {
					//e->pDisp=D0;
					e->pEnd=*GetVertex(P1);
					e->pStart= *GetVertex(P0);

				}else{
					e->pStart= *GetVertex(P1);
					e->pEnd=*GetVertex(P0);
					//e->pDisp=Point(0,0)-D0;
				}

				if (AddEdge(e)==-1) {
					// the JEdge is already there
					e->id=IsPortal(e->pStart.pPos, e->pEnd.pPos);
				}
				// caution: the ID is automatically assigned in the AddEdge method
				node->pPortals.push_back(e->id);
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

				//assert(node1!=-1);
				if(node1!=-1) { // we are having an egde

					JEdge* e= new JEdge();

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
					Point D1 = centroid0-P0;
					if (D0.Det(D1) < 0) {
						e->pEnd=*GetVertex(P1);
						//e->pDisp=D0;
						e->pStart= *GetVertex(P0);

					}else{
						e->pStart= *GetVertex(P1);
						e->pEnd=*GetVertex(P0);
						//e->pDisp=Point(0,0)-D0;
					}

					if (AddEdge(e)==-1) {
						// the JEdge is already there
						e->id=IsPortal(e->pStart.pPos, e->pEnd.pPos);
					}
					// caution: the ID is automatically assigned in the AddEdge method
					node->pPortals.push_back(e->id);
				}
				else
				{ // we are having an JObstacle
					JObstacle* o= new JObstacle();
					o->pNode0=node0;
					o->pNextObst=-1;

					//first attempt
					Point P0 = transitions[t]->GetPoint1();
					Point P1 = transitions[t]->GetPoint2();
					Point D0 = P1 - P0;
					Point D1 = centroid0-P0;
					if (D0.Det(D1) < 0) {
						//o->pDisp=D0;
						o->pEnd=*GetVertex(P1);
						o->pStart= *GetVertex(P0);

					}else{
						o->pStart= *GetVertex(P1);
						//o->pDisp=Point(0,0)-D0;
						o->pEnd=*GetVertex(P0);
					}

					if (AddObst(o)==-1) {
						// the JObstacle is already there
						o->id=IsObstacle(o->pStart.pPos, o->pEnd.pPos);
					}
					node->pObstacles.push_back(o->id);
				}

			}

			//determine the group based on the crossings
			if(crossings.size()==1 && transitions.size()==0){
				//JNode->pGroup="seat";
				node->pGroup=r->GetCaption();
			}else {
				if(crossings.size()==2){
					if(crossings[0]->Length()==crossings[1]->Length())
						node->pGroup="seats";
				}
			}

			//obstacles
			const vector<Wall>& walls = s->GetAllWalls();
			for (unsigned w = 0; w < walls.size(); w++) {

				const Point& centroid0 = s->GetCentroid();
				int node0 = s->GetUID();

				JObstacle* o= new JObstacle();
				o->pNode0=node0;
				o->pNextObst=-1;

				//first attempt
				Point P0 = walls[w].GetPoint1();
				Point P1 = walls[w].GetPoint2();
				Point D0 = P1 - P0;
				Point D1 = centroid0-P0;
				if (D0.Det(D1) < 0) {
					//o->pDisp=D0;
					o->pEnd=*GetVertex(P1);
					o->pStart= *GetVertex(P0);

				}else{
					o->pStart= *GetVertex(P1);
					//o->pDisp=Point(0,0)-D0;
					o->pEnd=*GetVertex(P0);
				}

				if (AddObst(o)==-1) {
					// the JEdge is already there
					o->id=IsObstacle(o->pStart.pPos, o->pEnd.pPos);
				}
				node->pObstacles.push_back(o->id);
			}
			AddNode(node);
			subroom_to_node[s->GetUID()]=node->id;
		}
	}

	// convexify the mesh
	Convexify();
	Finalize();

	std::sort(pNodes.begin(), pNodes.end(),JNode());

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

	//chain the obstacles
	for (unsigned int ob1 = 0; ob1 < pObst.size(); ob1++)
	{ continue; //FIXME
	for (unsigned int ob2 = 0; ob2 < pObst.size(); ob2++)
	{
		JObstacle* obst1 = pObst[ob1];
		JObstacle* obst2 = pObst[ob2];

		if (obst1->id == obst2->id)
			continue;
		int comVertex=obst1->GetCommonVertex(obst2);
		if(comVertex==-1)
			continue;

		if(obst1->pStart.id==comVertex)
		{
			obst2->pNextObst=obst1->id;
		}
		else
		{
			obst1->pNextObst=obst2->id;
		}

	}
	}
}

void NavMesh::DumpNode(int id) {
	JNode *nd=pNodes[id];

	std::cerr<<"ID: [ "<<endl;
	for(unsigned int i=0;i<nd->pHull.size();i++)
	{
		std::cerr<<nd->pHull[i].id<<" ";
	}
	std::cerr<<endl<<" ]"<<endl;
	//exit(0);
}

void NavMesh::Convexify() {

	//will hold the newly created elements
	std::vector<JVertex*> new_vertices;
	std::vector<JEdge*> new_edges;
	std::vector<JObstacle*> new_obsts;


	std::vector<JNode*> nodes_to_be_deleted;

	for (unsigned int n = 0; n < pNodes.size(); n++) {

		JNode* old_node = pNodes[n];
		if (old_node->IsClockwise()) {
			reverse(old_node->pHull.begin(), old_node->pHull.end());
		}

		if (old_node->IsConvex() == false) {

#ifdef _CGAL
			string group=old_node->pGroup;

			cout<<"convexifing:" <<group<< " ID: "<<old_node->id <<endl;

			//const char* myGroups[] = {"100","090","070","120","130","140","060"};
			//vector<string> nodes_to_plot (myGroups, myGroups + sizeof(myGroups) / sizeof(char*) );

			//if (IsElementInVector(nodes_to_plot, group) == true)
			//	continue;



			//schedule this JNode for deletion
			nodes_to_be_deleted.push_back(old_node);

			Polygon_2 polygon;
			Polygon_list partition_polys;
			Traits partition_traits;
			Validity_traits validity_traits;

			//create the CGAL structure
			for(unsigned int i=0;i<old_node->pHull.size();i++){
				double x=pVertices[old_node->pHull[i].id]->pPos.GetX() ;
				double y=pVertices[old_node->pHull[i].id]->pPos.GetY() ;
				polygon.push_back(Point_2(x, y));

			}

			//polygon
			try {
				if(polygon.is_clockwise_oriented()) polygon.reverse_orientation();

				//create the partitions
				CGAL::optimal_convex_partition_2(polygon.vertices_begin(),
						//CGAL::approx_convex_partition_2(polygon.vertices_begin(),
						polygon.vertices_end(), std::back_inserter(partition_polys),
						partition_traits);

				//check the created partitions
				assert(CGAL::partition_is_valid_2(polygon.vertices_begin(),
						polygon.vertices_end(), partition_polys.begin(),
						partition_polys.end(), validity_traits));


			}
			catch(const exception & e) {

				cout<<"JNode :" <<old_node->id <<" could not be converted" <<endl;
				cout<<" in Group: " <<old_node->pGroup <<endl;
				cout<<" Portals: " <<old_node->pPortals.size() <<endl;
				cout<<" Obstacles: " <<old_node->pObstacles.size() <<endl;
				problem_nodes.push_back(old_node->id);
				cout<<e.what()<<endl;
				Triangulate(old_node);
//				exit(EXIT_FAILURE);
				//return;
				//continue;
			}

			//make the changes to the nav mesh
			for (Polygon_iterator pit = partition_polys.begin();
					pit != partition_polys.end(); ++pit) {


				JNode* new_node = new JNode();
				new_node->pGroup = old_node->pGroup;
				//to get a correct ID
				AddNode(new_node);
				new_nodes.push_back(new_node);

				Point_2 c2 =CGAL::centroid(pit->vertices_begin(),pit->vertices_end(),CGAL::Dimension_tag<0>());
				new_node->pCentroid= Point(c2.x(),c2.y());

				new_node->pNormalVec[0]=0;
				new_node->pNormalVec[1]=0;
				new_node->pNormalVec[2]=0;

				for (Vertex_iterator vit = pit->vertices_begin();
						vit != pit->vertices_end(); ++vit) {
					new_node->pHull.push_back(*(GetVertex(Point(vit->x(), vit->y()))));
				}

				for (Edge_iterator eit=pit->edges_begin();eit!=pit->edges_end();++eit){

					Point P0  = Point (eit->start().x(), eit->start().y());
					Point P1  = Point (eit->end().x(), eit->end().y());


					int edge_id=IsPortal(P0,P1);
					if(edge_id != -1){
						new_node->pPortals.push_back(edge_id);
						JEdge* e = pEdges[edge_id];

						// check if all JEdge information are present
						// in particular one the second node_id might not be set yet
						//assert(e->pNode1==-1);
						if(e->pNode1 == -1){
							e->pNode1=new_node->id;
							if (e->pNode0 > e->pNode1){
								swap (e->pNode0, e->pNode1);
							}

							//new set the correct orientation
							//first attempt
							Point D0 = P1 - P0;
							Point centroid0=pNodes[e->pNode0]->pCentroid;
							if (D0.Det(centroid0) < 0) {
								e->pEnd=*GetVertex(P1);
								e->pStart= *GetVertex(P0);

							}else{
								e->pStart= *GetVertex(P1);
								e->pEnd=*GetVertex(P0);
							}

						}
					}

					int obstacle_id=IsObstacle(P0,P1);
					if(obstacle_id != -1){
						//std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
						new_node->pObstacles.push_back(obstacle_id);
						pObst[obstacle_id]->pNode0=new_node->id;
					}

					// this portal was newly created
					if ((obstacle_id==-1) && (edge_id==-1)){

						JEdge* e= new JEdge();
						e->pEnd=*GetVertex(P1);
						e->pStart= *GetVertex(P0);
						AddEdge(e);

						// add so we can get a correct ID
						e->pNode0=new_node->id;

						// caution: the ID is automatically assigned in the AddEdge method
						new_node->pPortals.push_back(e->id);

						//add for post processing later
						new_edges.push_back(e);

					}
				}
			}
#endif
		}
	}

#ifdef _DEBUG
	cout <<"before: " <<endl;
	cout << pNodes.size() <<" total nodes" <<endl;
	cout << new_nodes.size() <<" new nodes were created" <<endl;
	cout<< nodes_to_be_deleted.size()<<" nodes to be deleted"<<endl;
#endif

	// now post processing the newly created nodes

	for (unsigned int i=0;i<nodes_to_be_deleted.size();i++){
		JNode* node_to_delete = nodes_to_be_deleted[i];
		assert (node_to_delete->id != (pNodes.size() -1) && "Trying to remove the last JNode !");
		JNode* new_node = pNodes.back();
		pNodes.pop_back();

		//making the transformation

		for(unsigned int i=0;i<new_node->pObstacles.size();i++){
			pObst[new_node->pObstacles[i]]->pNode0=node_to_delete->id;
		}


		for(unsigned int i=0;i<new_node->pPortals.size();i++){

			if(pEdges[new_node->pPortals[i]]->pNode0==new_node->id){
				pEdges[new_node->pPortals[i]]->pNode0=node_to_delete->id;
			}
			else
			{
				pEdges[new_node->pPortals[i]]->pNode1=node_to_delete->id;
			}
		}

		new_node->id=node_to_delete->id;
		pNodes[node_to_delete->id]=new_node;

		cout<<"deleting JNode: "<<node_to_delete->id<<endl;
		//delete node_to_delete;
	}

#ifdef _DEBUG
	cout <<"after: " <<endl;
	cout << pNodes.size() <<" total nodes" <<endl;
	cout << new_nodes.size() <<" new nodes were created" <<endl;
	cout<< nodes_to_be_deleted.size()<<" nodes to be deleted"<<endl;
#endif
}


void NavMesh::WriteToFileTraVisTo(std::string fileName) {
	ofstream file(fileName.c_str());
	file.precision(2);
	file<<fixed;

	//Point centre (10299,2051);
	Point centre (0,0);
	double factor=100;

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
	//	int mynodes[] = {47, 30 ,38};
	int mynodes[] = {};
	//int mynodes[] = { 28, 27, 40};
	vector<int> nodes_to_plot (mynodes, mynodes + sizeof(mynodes) / sizeof(int) );


	//for (unsigned int n=0;n<new_nodes.size();n++){
	//	JNode* JNode=new_nodes[n];

	for (unsigned int n=0;n<pNodes.size();n++){
		JNode* JNode=pNodes[n];

		int node_id=JNode->id;
		if(nodes_to_plot.size()!=0)
			if (IsElementInVector(nodes_to_plot, node_id) == false)
				continue;
		//		if(problem_nodes.size()!=0)
		//			if (IsElementInVector(problem_nodes, node_id) == false)
		//				continue;

		//		if(JNode->pGroup!="090") continue;
		//		if(JNode->pPortals.size()<10) continue;
		//if(JNode->IsConvex()==true) continue;
		//if(JNode->IsClockwise()==true) continue;

		//		file<<"\t\t<label centerX=\""<<JNode->pCentroid.GetX()*factor -centre.pX<<"\" centerY=\""<<JNode->pCentroid.GetY()*factor-centre.pY<<"\" centerZ=\"0\" text=\""<<JNode->id <<"\" color=\"100\" />"<<endl;

		//		cout<<"size: "<< JNode->pHull.size()<<endl;
		//		std::sort(JNode->pHull.begin(), JNode->pHull.end());
		//		JNode->pHull.erase(std::unique(JNode->pHull.begin(), JNode->pHull.end()), JNode->pHull.end());
		//		cout<<"size: "<< JNode->pHull.size()<<endl;

		for(unsigned int i=0;i<JNode->pHull.size();i++){
			//double x=pVertices[JNode->pHull[i].id]->pPos.GetX()*factor -centre.pX;
			//double y=pVertices[JNode->pHull[i].id]->pPos.GetY()*factor -centre.pY;
			//file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
			//file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<JNode->pHull[i].id<<"\" color=\"20\" />"<<endl;

			// draw the convex hull
			//			unsigned int size= JNode->pHull.size();
			//			file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
			//			file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"20\" />"<<endl;
			//			double x1=pVertices[JNode->pHull[i%size].id]->pPos.GetX()*factor -centre.pX;
			//			double y1=pVertices[JNode->pHull[i%size].id]->pPos.GetY()*factor -centre.pY;
			//			double x2=pVertices[JNode->pHull[(i+1)%size].id]->pPos.GetX()*factor -centre.pX;
			//			double y2=pVertices[JNode->pHull[(i+1)%size].id]->pPos.GetY()*factor -centre.pY;
			//			file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
			//			file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
			//			file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
			//			file<<"\t\t</wall>"<<endl;
			//			cout.precision(2);
			//			cout<<fixed;
			//			printf("polygon.push_back(Point_2(%f, %f));\n",x1,y1);
		}
		file<<endl;
		//break;

		for(unsigned int i=0;i<pObst.size();i++){
			JObstacle* obst=pObst[i];

			if(obst->pNode0==node_id ){
				double x1=obst->pStart.pPos.GetX()*factor-centre.pX;
				double y1=obst->pStart.pPos.GetY()*factor-centre.pY;
				double x2=obst->pEnd.pPos.GetX()*factor-centre.pX;
				double y2=obst->pEnd.pPos.GetY()*factor-centre.pY;

				file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
				file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
				file<<"\t\t</wall>"<<endl;
			}

		}
		file<<endl;

		for(unsigned int i=0;i<pEdges.size();i++){
			JEdge* JEdge=pEdges[i];

			if(JEdge->pNode0==node_id || JEdge->pNode1==node_id){
				double x1=JEdge->pStart.pPos.GetX()*factor-centre.pX;
				double y1=JEdge->pStart.pPos.GetY()*factor-centre.pY;
				double x2=JEdge->pEnd.pPos.GetX()*factor-centre.pX;
				double y2=JEdge->pEnd.pPos.GetY()*factor-centre.pY;

				file<<"\t\t<door id = \""<<i<<"\">"<<endl;
				file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
				file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
				file<<"\t\t</door>"<<endl;
			}

		}


		//		for(unsigned int i=0;i<JNode->pObstacles.size();i++)
		//		{ continue;
		//			double x1=pObst[JNode->pObstacles[i]]->pStart.pPos.GetX()*FAKTOR;
		//			double y1=pObst[JNode->pObstacles[i]]->pStart.pPos.GetY()*FAKTOR;
		//			double x2=pObst[JNode->pObstacles[i]]->pEnd.pPos.GetX()*FAKTOR;
		//			double y2=pObst[JNode->pObstacles[i]]->pEnd.pPos.GetY()*FAKTOR;
		//
		//			file<<"\t\t<wall>"<<endl;
		//			file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
		//			file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
		//			file<<"\t\t</wall>"<<endl;
		//		}
		//
		//		file<<endl;
		//
		//		for(unsigned int i=0;i<JNode->pPortals.size();i++)
		//		{
		//			double x1=pEdges[JNode->pPortals[i]]->pStart.pPos.GetX()*FAKTOR;
		//			double y1=pEdges[JNode->pPortals[i]]->pStart.pPos.GetY()*FAKTOR;
		//			double x2=pEdges[JNode->pPortals[i]]->pEnd.pPos.GetX()*FAKTOR;
		//			double y2=pEdges[JNode->pPortals[i]]->pEnd.pPos.GetY()*FAKTOR;
		//
		//			file<<"\t\t<door>"<<endl;
		//			file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
		//			file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
		//			file<<"\t\t</door>"<<endl;
		//		}

		file<<endl;
	}

	file<<"\t</geometry>"<<endl;
	file.close();
}

void NavMesh::WriteToFileTraVisTo(std::string fileName, const std::vector<Point>& points) {
	ofstream file(fileName.c_str());
	file.precision(2);
	file<<fixed;

	//Point centre (10299,2051);
	Point centre (0,0);
	double factor=100;

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



	for(unsigned int i=0;i<points.size();i++){

		unsigned int size= points.size();
		double x1=points[ i%size].GetX()*factor -centre.pX;
		double y1=points[ i%size].GetY()*factor -centre.pY;
		double x2=points[ (i+1)%size].GetX()*factor -centre.pX;
		double y2=points[ (i+1)%size].GetY()*factor -centre.pY;

		//		draw the convex hull
		file<<" \t\t<sphere centerX=\""<<x1<<"\" centerY=\""<<y1<<"\" centerZ=\"0\" radius=\"150\" color=\"100\" />"<<endl;
		file<<"\t\t<label centerX=\""<<x1<<"\" centerY=\""<<y1<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"20\" />"<<endl;
		file<<"\t\t<label centerX=\""<<0.5*(x1+x2)<<"\" centerY=\""<<0.5*(y1+y2)<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"180\" />"<<endl;
		file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
		file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
		file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
		file<<"\t\t</wall>"<<endl;

		//		cout.precision(2);
		//		cout<<fixed;
		//		printf("polygon.push_back(Point_2(%f, %f));\n",x1,y1);
	}
	file<<endl;


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
	//file<<"# vertices section"<<endl;
	file<<pVertices.size()<<endl;
	for (unsigned int v=0;v<pVertices.size();v++){
		file<<"\t"<<pVertices[v]->pPos.GetX()<<" " <<pVertices[v]->pPos.GetY()<<endl;

	}

	//write the edges
	//file<<endl<<"# edges section"<<endl;
	file<<pEdges.size()<<endl;
	for (unsigned int e=0;e<pEdges.size();e++){
		//file<<pEdges[e]->pStart.pPos.GetX()<<" " <<pEdges[e]->pStart.pPos.GetY()<<endl;
		//file<<"\t"<<pEdges[e]->pDisp.GetX()<<" " <<pEdges[e]->pDisp.GetY()<<endl;
		file<<"\t";
		file<<pEdges[e]->pStart.id<<" " <<pEdges[e]->pEnd.id<<" ";
		file<<pEdges[e]->pNode0<<" " <<pEdges[e]->pNode1<<endl;
	}


	//write the obstacles
	//file<<endl<<"# Obstacles section"<<endl;
	file<<pObst.size()<<endl;
	for (unsigned int ob=0;ob<pObst.size();ob++){
		file<<"\t";
		file<<pObst[ob]->pStart.id<<" " <<pObst[ob]->pEnd.id<<" ";
		file<<pObst[ob]->pNode0<<" "<<pObst[ob]->pNextObst<<endl;
	}

	//write the nodes
	//file<<endl<<"# Nodes section"<<endl;

	std::map<string,int> ngroup_to_size;
	for (unsigned int n=0;n<pNodes.size();n++){
		ngroup_to_size[pNodes[n]->pGroup]++;
	}

	string previousGroup= pNodes[0]->pGroup;
	file<<endl<<previousGroup<<endl;
	file<<ngroup_to_size[previousGroup]<<"";

	for (unsigned int n=0;n<pNodes.size();n++){
		JNode* JNode=pNodes[n];
		string actualGroup=JNode->pGroup;
		if(actualGroup!=previousGroup){
			previousGroup=actualGroup;
			//file<<"# JNode group"<<endl;
			file<<endl<<previousGroup<<endl;
			file<<ngroup_to_size[previousGroup]<<"";
		}

		//assert(JNode->pObstacles.size()<20);
		//assert(JNode->pPortals.size()<20);
		//file<<JNode->id<<endl;
		file<<endl;
		file<<"\t"<<JNode->pCentroid.GetX()<<" "<<JNode->pCentroid.GetY()<<endl;
		file<<"\t"<<JNode->pHull.size()<<" ";
		for(unsigned int i=0;i<JNode->pHull.size();i++){
			file<<JNode->pHull[i].id<<" ";
		}
		file<<endl;
		file<<"\t"<<JNode->pNormalVec[0]<<" "<<JNode->pNormalVec[1]<<" "<<JNode->pNormalVec[2]<<endl;


		file<<"\t"<<JNode->pPortals.size()<<" ";
		for(unsigned int i=0;i<JNode->pPortals.size();i++){
			file<<JNode->pPortals[i]<<" ";
		}
		file<<endl;

		file<<"\t"<<JNode->pObstacles.size()<<" ";
		for(unsigned int i=0;i<JNode->pObstacles.size();i++){
			file<<JNode->pObstacles[i]<<" ";
		}

		file<<endl;
	}

	file.close();
}

int NavMesh::AddVertex(JVertex* v) {
	for (unsigned int vc = 0; vc < pVertices.size(); vc++) {
		if (pVertices[vc]->pPos.operator ==(v->pPos)) {
#ifdef _DEBUG
			cout << "JVertex already present:" << pVertices[vc]->id << endl;
#endif
			return -1;
		}
	}
	if (pVertices.size() == 0) {
		v->id = 0;
	} else {
		v->id = pVertices[pVertices.size() - 1]->id + 1;
	}
	pVertices.push_back(v);
	return v->id;
}

int NavMesh::AddEdge(JEdge* e) {
	int id = IsPortal(e->pStart.pPos, e->pEnd.pPos);

	if ((IsElementInVector(pEdges, e) == false) && (id == -1)) {
		if (pEdges.size() == 0) {
			e->id = 0;
		} else {
			e->id = pEdges[pEdges.size() - 1]->id + 1;
		}
		pEdges.push_back(e);
		return e->id;
	} else {
#ifdef _DEBUG
		cout << "JEdge already present:" << id << endl;
#endif
	}

	return -1;
}

int NavMesh::AddObst(JObstacle* o) {
	int id= IsObstacle(o->pStart.pPos, o->pEnd.pPos);

	if ( (IsElementInVector(pObst, o) == false) &&
			(id==-1 )){
		if (pObst.size() == 0) {
			o->id = 0;
		} else {
			o->id = pObst[pObst.size() - 1]->id + 1;
		}
		pObst.push_back(o);
		return o->id;
	} else {
#ifdef _DEBUG
		cout << "Obstacles already present:" << id << endl;
#endif
	}

	return -1;
}

int NavMesh::AddNode(JNode* node) {

	std::sort(node->pObstacles.begin(),node->pObstacles.end());
	std::sort(node->pPortals.begin(),node->pPortals.end());

	for(unsigned int n=0;n<pNodes.size();n++){
		if(*pNodes[n]==*node)  {
#ifdef _DEBUG
			cout << "JNode already present:" << node->id << endl;
#endif
			return -1;
		}
	}


	if (IsElementInVector(pNodes, node)) {

#ifdef _DEBUG
		cout << "JNode already present:" << node->id << endl;
#endif
		return -1;
	}

	if (pNodes.size() == 0) {
		node->id = 0;
	} else {
		node->id = pNodes[pNodes.size() - 1]->id + 1;
	}
	pNodes.push_back(node);
	return node->id;
}



NavMesh::JVertex* NavMesh::GetVertex(const Point& p) {

	for(unsigned int v=0;v<pVertices.size();v++){
		if(pVertices[v]->pPos.operator ==(p)){
			return pVertices[v];
		}
	}
#ifdef _DEBUG
	cout<<"JVertex not found: "<< p.GetX()<<":"<<p.GetY()<<endl;
	cout<<"Adding "<<endl;
	cout<<"pVertices.size()="<<pVertices.size()<<endl;
#endif
	JVertex* v = new JVertex();
	v->pPos= p;
	AddVertex(v);
	//this could lead to an infinite loop if the point cannot be added and cannot be found
	return GetVertex(p);

	//exit(EXIT_FAILURE);
}

int NavMesh::IsPortal(Point& p1, Point& p2) {

	for(unsigned int i=0;i<pEdges.size();i++){
		JEdge* JEdge=pEdges[i];

		if( (JEdge->pStart.pPos==p1) && (JEdge->pEnd.pPos==p2)){
			return JEdge->id;
		}

		if( (JEdge->pStart.pPos==p2) && (JEdge->pEnd.pPos==p1)){
			return JEdge->id;
		}
	}
	return -1;
}

void NavMesh::Finalize() {

#ifdef _CGAL
	cout<<"finalizing"<<endl;

	//	WriteToFileTraVisTo("arena_envelope.xml",envelope);

	//collect all possible vertices that form that envelope

	vector<Line> envelope;
	vector<Point> centroids;
	//centroids.push_back(Point(0,0));
	centroids.push_back(Point(60,40));
	centroids.push_back(Point(60,-40));
	centroids.push_back(Point(-60,40));
	centroids.push_back(Point(-60,-40));
	centroids.push_back(Point(00,-40));
	centroids.push_back(Point(00,40));
	centroids.push_back(Point(-30,-40));
	centroids.push_back(Point(30,40));
	centroids.push_back(Point(-30,40));
	centroids.push_back(Point(30,-40));
	centroids.push_back(Point(60,00));
	centroids.push_back(Point(-70,00));
	centroids.push_back(Point(-60,-20));


	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* r = pBuilding->GetRoom(i);
		string caption = r->GetCaption();

		//skip the virtual room containing the complete geometry
		if(r->GetCaption()=="outside") continue;
		if(r->GetZPos()>6) continue;
		const Point& centroid0 = Point(0,0);

		for (int k = 0; k < r->GetAnzSubRooms(); k++) {
			SubRoom* s = r->GetSubRoom(k);

			//walls
			const vector<Wall>& walls = s->GetAllWalls();

			for (unsigned w = 0; w < walls.size(); w++) {

				bool skip=false;
				for(unsigned int i=0;i<centroids.size();i++){
					if(walls[w].DistTo(centroids[i])<25) skip=true;
				}
				if(skip==true) continue;

				//first attempt
				Point P0 = walls[w].GetPoint1();
				Point P1 = walls[w].GetPoint2();
				Point D0 = P1 - P0;
				Point D1 = centroid0-P0;
				if (D0.Det(D1) < 0) {
					envelope.push_back(Line(P0, P1));
				}else{
					envelope.push_back(Line(P1, P0));
				}
			}


			const vector<Transition*>& transitions = s->GetAllTransitions();
			for (unsigned t = 0; t < transitions.size(); t++) {

				if(transitions[t]->GetSubRoom2() != NULL) continue;

				bool skip=false;
				for(unsigned int i=0;i<centroids.size();i++){
					if(transitions[t]->DistTo(centroids[i])<25) skip=true;
				}
				if(skip==true) continue;

				//first attempt
				Point P0 = transitions[t]->GetPoint1();
				Point P1 = transitions[t]->GetPoint2();
				Point D0 = P1 - P0;
				Point D1 = centroid0-P0;
				if (D0.Det(D1) < 0) {
					envelope.push_back(Line(P0, P1));
				}else{
					envelope.push_back(Line(P1, P0));
				}
			}
		}
	}


	//link those vertices
	vector<Point> Hull;
	Hull.push_back(envelope[envelope.size()-1].GetPoint1());
	Hull.push_back(envelope[envelope.size()-1].GetPoint2());
	envelope.pop_back();

	while(envelope.empty()==false){
		for(unsigned int i=0;i<envelope.size();i++){
			if(envelope[i].GetPoint1()==Hull[Hull.size()-1]){
				Hull.push_back(envelope[i].GetPoint2());
				envelope.erase(envelope.begin()+i);
			}else if(envelope[i].GetPoint2()==Hull[Hull.size()-1])
			{
				Hull.push_back(envelope[i].GetPoint1());
				envelope.erase(envelope.begin()+i);
			}
		}
	}

	//eject the last point which is a duplicate.
	Hull.pop_back();

	//print for some check
	//WriteToFileTraVisTo("arena_envelope.xml",Hull);
	//exit(0);
	//now check the polygon with holes.

	//	{
	//		ofstream myfile ("mypoints.pts");
	//		if (myfile.is_open())
	//		{
	//			//quick testing
	//			for(unsigned int i=0;i<Hull2.size();i++){
	//				myfile <<"P "<<Hull2[i].pX <<" "<<Hull2[i].pY<<endl;
	//			}
	//			myfile <<"H "<<Hull[0].pX <<" "<<Hull[0].pY<<endl;
	//			for(unsigned int i=1;i<Hull.size();i++){
	//				myfile <<"P "<<Hull[i].pX <<" "<<Hull[i].pY<<endl;
	//			}
	//		}
	//
	//	}
	//WriteToFileTraVisTo("arena_envelope.xml",Hull);

	//first polygon
	Polygon_2 polygon2;
	Polygon_2 holesP[1];


	for(unsigned int i=0;i<Hull.size();i++){
		holesP[0].push_back(Point_2(Hull[i].pX,Hull[i].pY));
	}

	vector<Point> Hull2=pBuilding->GetRoom("outside")->GetSubRoom(0)->GetPolygon();
	for(unsigned int i=0;i<Hull2.size();i++){
		polygon2.push_back(Point_2(Hull2[i].pX,Hull2[i].pY));
	}

	if(holesP[0].is_clockwise_oriented())holesP[0].reverse_orientation();
	if(polygon2.is_clockwise_oriented())polygon2.reverse_orientation();

	assert(holesP[0].is_counterclockwise_oriented());
	assert(polygon2.is_counterclockwise_oriented());
	assert(holesP[0].is_simple());
	assert(polygon2.is_simple());

	{
		cout<<"performing the triangulation"<<endl;
		DTriangulation* tri= new DTriangulation();
		tri->SetOuterPolygone(Hull);
		tri->AddHole(Hull2);
		tri->Triangulate();
		vector<p2t::Triangle*> triangles=tri->GetTriangles();
		cout<<"size:"<<triangles.size()<<endl;

		//		CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 100, 100, 100));
		//		gv.set_line_width(4);
		//		gv.set_trace(true);
		//		gv.set_bg_color(CGAL::Color(0, 200, 200));
		//		// gv.clear();
		//
		//		// use different colors, and put a few sleeps/clear.
		//		gv << CGAL::BLUE;
		//		gv.set_wired(true);

		for(unsigned int t=0;t<triangles.size();t++){
			p2t::Triangle* tr =triangles[t];

			//			Point_2 P0  = Point_2 (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
			//			Point_2 P1  = Point_2 (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
			//			Point_2 P2  = Point_2 (tr->GetPoint(2)->x,tr->GetPoint(2)->y);
			//			gv << Segment_2(P0,P1);
			//			gv << Segment_2(P1,P2);
			//			gv << Segment_2(P0,P2);

			Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
			Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
			Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

			//create the new nodes
			{

				JNode* new_node = new JNode();
				new_node->pGroup = "outside";
				//to get a correct ID
				AddNode(new_node);
				new_nodes.push_back(new_node);
				new_node->pCentroid= (P0+P1+P2)*(1/3);

				new_node->pNormalVec[0]=0;
				new_node->pNormalVec[1]=0;
				new_node->pNormalVec[2]=0;

				// Points are by default counterclockwise
				new_node->pHull.push_back(*(GetVertex(P0)));
				new_node->pHull.push_back(*(GetVertex(P1)));
				new_node->pHull.push_back(*(GetVertex(P2)));


				for (int index=0;index<3;index++){

					Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
					Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);


					int edge_id=IsPortal(P0,P1);
					if(edge_id != -1){
						new_node->pPortals.push_back(edge_id);
						JEdge* e = pEdges[edge_id];

						// check if all JEdge information are present
						// in particular one the second node_id might not be set yet
						//assert(e->pNode1==-1);
						if(e->pNode1 == -1){
							e->pNode1=new_node->id;
							if (e->pNode0 > e->pNode1){
								swap (e->pNode0, e->pNode1);
							}

							//new set the correct orientation
							//first attempt
							Point D0 = P1 - P0;
							Point centroid0=pNodes[e->pNode0]->pCentroid;
							if (D0.Det(centroid0) < 0) {
								e->pEnd=*GetVertex(P1);
								e->pStart= *GetVertex(P0);

							}else{
								e->pStart= *GetVertex(P1);
								e->pEnd=*GetVertex(P0);
							}

						}
					}

					int obstacle_id=IsObstacle(P0,P1);
					if(obstacle_id != -1){
						//std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
						new_node->pObstacles.push_back(obstacle_id);
						pObst[obstacle_id]->pNode0=new_node->id;
					}

					// this portal was newly created
					if ((obstacle_id==-1) && (edge_id==-1)){

						JEdge* e= new JEdge();
						e->pEnd=*GetVertex(P1);
						e->pStart= *GetVertex(P0);
						AddEdge(e);

						// add so we can get a correct ID
						e->pNode0=new_node->id;

						// caution: the ID is automatically assigned in the AddEdge method
						new_node->pPortals.push_back(e->id);

					}
				}
			}
		}
	}
	//	//Insert the polyons into a constrained triangulation

	//	CDT cdt;
	//	insert_polygon(cdt,holesP[0]);
	//	insert_polygon(cdt,polygon2);
	//
	//	//Mark facets that are inside the domain bounded by the polygon
	//	mark_domains(cdt);
	//
	//	int count=0;
	//	for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
	//			fit!=cdt.finite_faces_end();++fit)
	//	{
	//		if ( fit->info().in_domain() ) ++count;
	//	}

	//cdt.draw_triangulation(std::cout);

	//
	//	CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 100, 100, 100));
	//	gv.set_line_width(4);
	//	gv.set_trace(true);
	//	gv.set_bg_color(CGAL::Color(0, 200, 200));
	//	// gv.clear();
	//
	//	// use different colors, and put a few sleeps/clear.
	//	gv << CGAL::BLUE;
	//	gv.set_wired(true);
	//
	//
	//	for (Edge_iterator eit=holesP[0].edges_begin();eit!=holesP[0].edges_end();++eit){
	//		Point_2 P0  = Point_2 (eit->start().x(), eit->start().y());
	//		Point_2 P1  = Point_2 (eit->end().x(), eit->end().y());
	//		gv << Segment_2(P0,P1);
	//	}
	//
	//	for (Edge_iterator eit=polygon2.edges_begin();eit!=polygon2.edges_end();++eit){
	//		Point_2 P0  = Point_2 (eit->start().x(), eit->start().y());
	//		Point_2 P1  = Point_2 (eit->end().x(), eit->end().y());
	//		gv << Segment_2(P0,P1);
	//	}
	//
	//	CDT::Finite_faces_iterator it;
	//	int stop=0;
	//	for (it = cdt.finite_faces_begin(); it != cdt.finite_faces_end(); it++)
	//	{
	//		std::cout << cdt.triangle(it) << std::endl;
	//		gv << cdt.triangle(it) ;
	//		if (stop++ > 10) break;
	//	}
	//
	//	//gv << cdt;
	//
	//	getc(stdin);

	//exit(0);

#endif //_CGAL

	cout<<"done with finalization"<<endl;
}


void NavMesh::Triangulate(SubRoom* sub) {

	//vertices
	vector<Point> outerHull= sub->GetPolygon();

	// Vertices
	for (unsigned int p = 0; p < outerHull.size(); p++) {
		JVertex* v = new JVertex();
		v->pPos= outerHull[p];
		if(AddVertex(v)==-1) {
			delete v;
		}
	}


	vector<vector<Point> >holes;
	const vector<Obstacle*>& obstacles= sub->GetAllObstacles();

	for(unsigned int p=0;p<obstacles.size();p++){
		holes.push_back(obstacles[p]->GetPolygon());
	}


#ifdef _CGAL

	//perform some checks
	Polygon_2 polygon;
	Polygon_2 holesP[holes.size()];

	for(unsigned int i=0;i<outerHull.size();i++){
		polygon.push_back(Point_2(outerHull[i].pX,outerHull[i].pY));
	}
	assert(polygon.is_simple());
	if(polygon.is_clockwise_oriented()){
		std::reverse(outerHull.begin(), outerHull.end());
		polygon.reverse_orientation();
	}
	assert(polygon.is_counterclockwise_oriented());


	for(unsigned int i=0;i<holes.size();i++){
		for(unsigned int j=0;j<holes[i].size();j++){
			holesP[i].push_back(Point_2(holes[j][i].pX,holes[j][i].pY));
		}

		if(holesP[i].is_clockwise_oriented()) {
			holesP[i].reverse_orientation();
			std::reverse(holes[i].begin(), holes[i].end());
		}
		assert(holesP[i].is_counterclockwise_oriented());
		assert(holesP[i].is_simple());
	}

#endif // _CGAL


	DTriangulation* tri= new DTriangulation();
	tri->SetOuterPolygone(outerHull);

	//add the holes
	for(unsigned int i=0;i<holes.size();i++){
		tri->AddHole(holes[i]);
	}

	tri->Triangulate();

	vector<p2t::Triangle*> triangles=tri->GetTriangles();
	//cout<<"size:"<<triangles.size()<<endl;

	for(unsigned int t=0;t<triangles.size();t++)
	{
		p2t::Triangle* tr =triangles[t];

		Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
		Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
		Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

		//create the new nodes
		JNode* new_node = new JNode();
		new_node->pGroup = "outside";
		//to get a correct ID
		AddNode(new_node);
		new_nodes.push_back(new_node);
		new_node->pCentroid= (P0+P1+P2)*(1/3);

		new_node->pNormalVec[0]=0;
		new_node->pNormalVec[1]=0;
		new_node->pNormalVec[2]=pBuilding->GetRoom(sub->GetRoomID())->GetZPos();

		// Points are by default counterclockwise
		new_node->pHull.push_back(*(GetVertex(P0)));
		new_node->pHull.push_back(*(GetVertex(P1)));
		new_node->pHull.push_back(*(GetVertex(P2)));


		for (int index=0;index<3;index++){

			Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
			Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

			int edge_id=IsPortal(P0,P1);
			if(edge_id != -1){
				new_node->pPortals.push_back(edge_id);
				JEdge* e = pEdges[edge_id];

				// check if all JEdge information are present
				// in particular one the second node_id might not be set yet
				//assert(e->pNode1==-1);
				if(e->pNode1 == -1){
					e->pNode1=new_node->id;
					if (e->pNode0 > e->pNode1){
						swap (e->pNode0, e->pNode1);
					}

					//new set the correct orientation
					//first attempt
					Point D0 = P1 - P0;
					Point centroid0=pNodes[e->pNode0]->pCentroid;
					if (D0.Det(centroid0) < 0) {
						e->pEnd=*GetVertex(P1);
						e->pStart= *GetVertex(P0);

					}else{
						e->pStart= *GetVertex(P1);
						e->pEnd=*GetVertex(P0);
					}
				}
			}

			int obstacle_id=IsObstacle(P0,P1);
			if(obstacle_id != -1){
				//std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
				new_node->pObstacles.push_back(obstacle_id);
				pObst[obstacle_id]->pNode0=new_node->id;
			}

			// this portal was newly created
			if ((obstacle_id==-1) && (edge_id==-1)){

				JEdge* e= new JEdge();
				e->pEnd=*GetVertex(P1);
				e->pStart= *GetVertex(P0);
				AddEdge(e);

				// add so we can get a correct ID
				e->pNode0=new_node->id;

				// caution: the ID is automatically assigned in the AddEdge method
				new_node->pPortals.push_back(e->id);
			}
		}
	}
}

void NavMesh::Triangulate(JNode* node) {

	if(node->IsConvex()) return;

	vector<Point> outerHull;

	for(unsigned int h=0;h<node->pHull.size();h++){
		outerHull.push_back(node->pHull[h].pPos);
	}

	DTriangulation* tri= new DTriangulation();
	tri->SetOuterPolygone(outerHull);

	//no hole
	//tri->AddHole(Hull2);
	tri->Triangulate();
	vector<p2t::Triangle*> triangles=tri->GetTriangles();
	cout<<"size:"<<triangles.size()<<endl;


	for(unsigned int t=0;t<triangles.size();t++){
		p2t::Triangle* tr =triangles[t];

		Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
		Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
		Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

		//create the new nodes
		JNode* new_node = new JNode();
		new_node->pGroup = node->pGroup;
		//to get a correct ID
		AddNode(new_node);
		new_nodes.push_back(new_node);
		new_node->pCentroid= (P0+P1+P2)*(1/3);

		new_node->pNormalVec[0]=node->pNormalVec[0];
		new_node->pNormalVec[1]=node->pNormalVec[1];
		new_node->pNormalVec[2]=node->pNormalVec[2];

		// Points are by default counterclockwise
		new_node->pHull.push_back(*(GetVertex(P0)));
		new_node->pHull.push_back(*(GetVertex(P1)));
		new_node->pHull.push_back(*(GetVertex(P2)));


		for (int index=0;index<3;index++){

			Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
			Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

			int edge_id=IsPortal(P0,P1);
			if(edge_id != -1){
				new_node->pPortals.push_back(edge_id);
				JEdge* e = pEdges[edge_id];

				// check if all JEdge information are present
				// in particular one the second node_id might not be set yet
				//assert(e->pNode1==-1);
				if(e->pNode1 == -1){
					e->pNode1=new_node->id;
					if (e->pNode0 > e->pNode1){
						swap (e->pNode0, e->pNode1);
					}

					//new set the correct orientation
					//first attempt
					Point D0 = P1 - P0;
					Point centroid0=pNodes[e->pNode0]->pCentroid;
					if (D0.Det(centroid0) < 0) {
						e->pEnd=*GetVertex(P1);
						e->pStart= *GetVertex(P0);

					}else{
						e->pStart= *GetVertex(P1);
						e->pEnd=*GetVertex(P0);
					}
				}
			}

			int obstacle_id=IsObstacle(P0,P1);
			if(obstacle_id != -1){
				//std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
				new_node->pObstacles.push_back(obstacle_id);
				pObst[obstacle_id]->pNode0=new_node->id;
			}

			// this portal was newly created
			if ((obstacle_id==-1) && (edge_id==-1)){

				JEdge* e= new JEdge();
				e->pEnd=*GetVertex(P1);
				e->pStart= *GetVertex(P0);
				AddEdge(e);

				// add so we can get a correct ID
				e->pNode0=new_node->id;

				// caution: the ID is automatically assigned in the AddEdge method
				new_node->pPortals.push_back(e->id);
			}
		}
	}
}

int NavMesh::IsObstacle(Point& p1, Point& p2) {

	for(unsigned int i=0;i<pObst.size();i++){
		JObstacle* obst=pObst[i];

		if( (obst->pStart.pPos==p1) && (obst->pEnd.pPos==p2)){
			return obst->id;
		}

		if( (obst->pStart.pPos==p2) && (obst->pEnd.pPos==p1)){
			return obst->id;
		}
	}

	return -1;
}

void NavMesh::WriteScenario() {
	WriteBehavior();
	WriteViewer();
	WriteStartPosition();
}

void NavMesh::WriteBehavior() {

}

void NavMesh::WriteViewer() {

}

void NavMesh::WriteStartPosition() {
}
