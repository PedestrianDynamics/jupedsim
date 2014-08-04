/**
 * \file        NavMesh.cpp
 * \date        Oct 29, 2012
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/


#include "NavMesh.h"

#define _USE_MATH_DEFINES
#include <math.h>
//#include "../MCD/GeomPoly.h"
//#include "../MCD/GeomVector.h"
//#include "../MCD/AlgorithmMCD.h"
#include "DTriangulation.h"
#include "../pedestrian/PedDistributor.h"
#include "../geometry/Obstacle.h"
#include "../geometry/SubRoom.h"
#include "../IO/OutputHandler.h"

//#define _DEBUG 1

using namespace std;


NavMesh::NavMesh(Building* b)
{
     _building=b;
}

NavMesh::~NavMesh()
{

     for (unsigned int i = 0; i < _vertices.size(); i++)
          delete _vertices[i];

     for (unsigned int i = 0; i < _edges.size(); i++)
          delete _edges[i];

     for (unsigned int i = 0; i < _obst.size(); i++)
          delete _obst[i];

     for (unsigned int i = 0; i < _nodes.size(); i++)
          delete _nodes[i];

}

void NavMesh::BuildNavMesh()
{

     //compute the equations of the plane first.
     ComputePlanesEquation();


     std::map<int,int> subroom_to_node;
     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* r = _building->GetRoom(i);
          string caption = r->GetCaption();

          //skip the virtual room containing the complete geometry
          //if(r->GetCaption()=="outside") continue;

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
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
               //setting the node equation. important for real 3D informations
               const double* ABC = s->GetPlanEquation();
               node->pNormalVec[0]=ABC[0];
               node->pNormalVec[1]=ABC[1];
               node->pNormalVec[2]=ABC[2];
               //                      ComputePlaneEquation(s,node->pNormalVec);

               for (unsigned int p = 0; p < pol.size(); p++) {
                    node->pHull.push_back(*(GetVertex(pol[p])));
               }


               //edges are transitions and crossings
               const vector<Crossing*>& crossings = s->GetAllCrossings();
               for (unsigned c = 0; c < crossings.size(); c++) {

                    JEdge* e= new JEdge();
                    int node0 = crossings[c]->GetSubRoom1()->GetUID();
                    int node1 = crossings[c]->GetSubRoom2()->GetUID();
                    Point P0 = crossings[c]->GetPoint1();
                    Point P1 = crossings[c]->GetPoint2();

                    assert(node0!=node1);
                    e->pNode0=node0;
                    e->pNode1=node1;
                    e->pEnd=*GetVertex(P1);
                    e->pStart= *GetVertex(P0);


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


                    if(transitions[t]->IsOpen()==true ) { // we are having an egde
                         //                              if(node1!=-1){

                         JEdge* e= new JEdge();

                         e->pNode0=node0;
                         e->pNode1=node1;
                         assert(node0!=node1);

                         //first attempt
                         Point P0 = transitions[t]->GetPoint1();
                         Point P1 = transitions[t]->GetPoint2();
                         e->pEnd=*GetVertex(P1);
                         e->pStart= *GetVertex(P0);

                         //TODO: release e memory
                         if (AddEdge(e)==-1) {
                              // the JEdge is already there
                              e->id=IsPortal(e->pStart.pPos, e->pEnd.pPos);
                         }
                         // caution: the ID is automatically assigned in the AddEdge method
                         node->pPortals.push_back(e->id);

                         if(e->id==1766) {
                              DumpEdge(111);
                              cout<<"name: " <<transitions[t]->GetCaption()<<endl;
                              cout<<"room 1: " <<transitions[t]->GetRoom1()->GetCaption()<<endl;
                              cout<<"room2: " <<transitions[t]->GetRoom2()->GetCaption()<<endl;
                              assert(0);
                         }
                    } else {
                         // we are having an JObstacle
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

                         } else {
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
               if(crossings.size()==1 && transitions.size()==0) {
                    //JNode->pGroup="seat";
                    node->pGroup=r->GetCaption();
               } else {
                    if(crossings.size()==2) {
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

                    } else {
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

               subroom_to_node[s->GetUID()]=node->id;

               if(r->GetCaption()!="outside") {
                    AddNode(node);
                    map_node_to_subroom[node->id]=make_pair(r->GetID(),s->GetSubRoomID());
               } else {
                    delete node;
               }
          }
     }

     // convexify the mesh
     Convexify();
     //Triangulate(pNodes[pBuilding->GetRoom("030")->GetSubRoom(0)->GetUID()]);
     //Triangulate(pNodes[pBuilding->GetRoom("040a")->GetSubRoom(0)->GetUID()]);
     //Triangulate(pNodes[pBuilding->GetRoom("030a")->GetSubRoom(0)->GetUID()]);
     //Finalize();
     FinalizeAlphaShape();
     //WriteToFileTraVisTo("promenade.nav.xml", pNodes[364]); exit(0);
     //WriteToFileTraVisTo("promenade.nav.xml");
     //cout<<"groupe:"<<pNodes[365]->pGroup<<endl;
     //cout<<"obst:"<<pNodes[1409]->pObstacles.size()<<endl;
     //DumpObstacle(pNodes[1409]->pObstacles[0]);
     //DumpNode(2341);
     //DumpEdge(9);
     UpdateNodes();
     Test();

     std::sort(_nodes.begin(), _nodes.end(),JNode());

     //      doing the mapping
     for(unsigned int i=0; i<_nodes.size(); i++) {
          subroom_to_node[_nodes[i]->id]=i;
          _nodes[i]->id=i;
     }


     //normalizing the IDs
     for (unsigned int e=0; e<_edges.size(); e++) {
          if(subroom_to_node.count(_edges[e]->pNode0)==0) {
               cout<<"Node 0 id (edge): "<< _edges[e]->pNode0<<" not in the map"<<endl;
               exit(0);
          }
          if(subroom_to_node.count(_edges[e]->pNode1)==0) {
               cout<<"Node 1 id (edge): "<< _edges[e]->pNode1<<" not in the map"<<endl;
               exit(0);
          }

          _edges[e]->pNode0=subroom_to_node[_edges[e]->pNode0];
          _edges[e]->pNode1=subroom_to_node[_edges[e]->pNode1];
     }

     for (unsigned int ob=0; ob<_obst.size(); ob++) {
          if(subroom_to_node.count(_obst[ob]->pNode0)==0) {
               cout<<"Node 0 id (obst): "<< _obst[ob]->pNode0<<" not in the map"<<endl;
               exit(0);
          }
          _obst[ob]->pNode0=subroom_to_node[_obst[ob]->pNode0];
     }

     //chain the obstacles
     for (unsigned int ob1 = 0; ob1 < _obst.size(); ob1++) {
          continue; //FIXME
          for (unsigned int ob2 = 0; ob2 < _obst.size(); ob2++) {
               JObstacle* obst1 = _obst[ob1];
               JObstacle* obst2 = _obst[ob2];

               if (obst1->id == obst2->id)
                    continue;
               int comVertex=obst1->GetCommonVertex(obst2);
               if(comVertex==-1)
                    continue;

               if(obst1->pStart.id==comVertex) {
                    obst2->pNextObst=obst1->id;
               } else {
                    obst1->pNextObst=obst2->id;
               }

          }
     }

     Log->Write("INFO:\tMesh successfully generated!\n");
     //      DumpNode(72);
     //      DumpEdge(66);
     //      exit(0);
}

void NavMesh::DumpNode(int id)
{
     JNode *nd=_nodes[id];


     std::cerr<<endl<<"Node ID: "<<id<<endl;
     std::cerr<<"Hull ID: [ "<<endl;
     for(unsigned int i=0; i<nd->pHull.size(); i++) {
          std::cerr<<nd->pHull[i].id<<" ";
     }
     std::cerr<<endl<<" ]"<<endl;

     std::cerr<<"Obstacles ID: ["<<endl;
     for( unsigned int i=0; i<nd->pObstacles.size(); i++) {
          std::cerr<<nd->pObstacles[i]<<" ";
     }
     std::cerr<<endl<<" ]"<<endl;

     std::cerr<<"Portals ID: ["<<endl;
     for( unsigned int i=0; i<nd->pPortals.size(); i++) {
          std::cerr<<nd->pPortals[i]<<" ";
     }
     std::cerr<<endl<<" ]"<<endl<<endl;

}

void NavMesh::DumpEdge(int id)
{
     JEdge* e= _edges[id];
     std::cerr<<endl<<"Edge: "<<endl;
     std::cerr<<"id: "<<e->id<<endl;
     std::cerr<<"node 0: "<<e->pNode0<<endl;
     std::cerr<<"node 1: "<<e->pNode1<<endl<<endl;
}

void NavMesh::DumpObstacle(int id)
{
     JObstacle* o= _obst[id];
     std::cerr<<endl<<"Obstacle: "<<endl;
     std::cerr<<"id: "<<o->id<<endl;
     std::cerr<<"node 0: "<<o->pNode0<<endl<<endl;

}

void NavMesh::Convexify()
{

     Log->Write("INFO:\tGenerating the navigation mesh!");
     for (unsigned int n = 0; n < _nodes.size(); n++) {

          JNode* old_node = _nodes[n];
          if (old_node->IsClockwise()) {
               reverse(old_node->pHull.begin(), old_node->pHull.end());
          }

          if (old_node->IsConvex() == false) {

               Triangulate(old_node);
          }
     }
     Log->Write("INFO:\t...Done!");

     /*
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

     #ifdef _DEBUG
                     cout<<"convexifing:" <<group<< " ID: "<<old_node->id <<endl;
     #endif
     //                      const char* myGroups[] = {"030"};
     //                      vector<string> nodes_to_plot (myGroups, myGroups + sizeof(myGroups) / sizeof(char*) );
     //
     //                      if (IsElementInVector(nodes_to_plot, group) == false)
     //                              continue;

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

                             cout<<"node :" <<old_node->id <<" could not be converted" <<endl;
                             cout<<" in Group: " <<old_node->pGroup <<endl;
                             cout<<" Portals: " <<old_node->pPortals.size() <<endl;
                             cout<<" Obstacles: " <<old_node->pObstacles.size() <<endl;
                             problem_nodes.push_back(old_node->id);
                             cout<<e.what()<<endl;
                             //Triangulate(old_node);
                             //exit(EXIT_FAILURE);
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

                             new_node->pNormalVec[0]=old_node->pNormalVec[0];
                             new_node->pNormalVec[1]=old_node->pNormalVec[1];
                             new_node->pNormalVec[2]=old_node->pNormalVec[2];

                             for (Vertex_iterator vit = pit->vertices_begin();
                                             vit != pit->vertices_end(); ++vit) {
                                     new_node->pHull.push_back(*(GetVertex(Point(vit->x(), vit->y()))));
                             }


                             if(new_node->IsClockwise()){
                                     std::reverse(new_node->pHull.begin(), new_node->pHull.end());
                             }

                             for (Edge_iterator eit=pit->edges_begin();eit!=pit->edges_end();++eit){

                                     Point P0  = Point (eit->start().x(), eit->start().y());
                                     Point P1  = Point (eit->end().x(), eit->end().y());


                                     int edge_id=IsPortal(P0,P1);
                                     if(edge_id != -1){
                                             new_node->pPortals.push_back(edge_id);
                                             JEdge* e = pEdges[edge_id];

                                             //invalidate the node
                                             e->pNode0=-1;
                                             e->pNode1=-1;
                                     }

                                     int obstacle_id=IsObstacle(P0,P1);
                                     if(obstacle_id != -1){
                                             //std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
                                             new_node->pObstacles.push_back(obstacle_id);
                                             pObst[obstacle_id]->pNode0=-1;
                                     }

                                     // this portal was newly created
                                     if ((obstacle_id==-1) && (edge_id==-1)){

                                             JEdge* e= new JEdge();
                                             e->pEnd=*GetVertex(P1);
                                             e->pStart= *GetVertex(P0);
                                             AddEdge(e);

                                             //invalidate the node
                                             e->pNode0=-1;
                                             e->pNode1=-1;

                                             // caution: the ID is automatically assigned in the AddEdge method
                                             new_node->pPortals.push_back(e->id);
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

     UpdateEdges();
     UpdateObstacles();


     // now post processing the newly created nodes
     for (unsigned int i=0;i<nodes_to_be_deleted.size();i++){
             JNode* node_to_delete = nodes_to_be_deleted[i];

             JNode* new_node = pNodes.back();
             pNodes.pop_back();

             assert (node_to_delete->id != new_node->id && "Trying to remove the last node !");

             //making the transformation

             for(unsigned int i=0;i<new_node->pObstacles.size();i++){
                     pObst[new_node->pObstacles[i]]->pNode0=node_to_delete->id;
             }


             for(unsigned int i=0;i<new_node->pPortals.size();i++){
                     JEdge* e= pEdges[new_node->pPortals[i]];

     //                      if(e->pNode0==node_to_delete->id || e->pNode1==node_to_delete->id){
     //
     //                      }else{
     //
     //
     //                              if(e->pNode0==new_node->id){
     //                                      e->pNode0=node_to_delete->id;
     //                              }
     //                                              else if(e->pNode1==new_node->id)
     //                              //else
     //                              {
     //                                      e->pNode1=node_to_delete->id;
     //                              }
     //                      }

                     if(pEdges[new_node->pPortals[i]]->pNode0==new_node->id){
                             pEdges[new_node->pPortals[i]]->pNode0=node_to_delete->id;
                     }
     //                      else if(pEdges[new_node->pPortals[i]]->pNode1==new_node->id)
                     else
                     {
                             pEdges[new_node->pPortals[i]]->pNode1=node_to_delete->id;
                     }
             }

             new_node->id=node_to_delete->id;
             pNodes[node_to_delete->id]=new_node;

     #ifdef _DEBUG
             cout<<"deleting node: "<<node_to_delete->id<<endl;
     #endif
             delete node_to_delete;
     }

     #ifdef _DEBUG
     cout <<"after: " <<endl;
     cout << pNodes.size() <<" total nodes" <<endl;
     cout << new_nodes.size() <<" new nodes were created" <<endl;
     cout<< nodes_to_be_deleted.size()<<" nodes to be deleted"<<endl;
     #endif


     //final cleaning

     UpdateEdges();
     UpdateObstacles();

     //exit(0);


      */
}



void NavMesh::WriteToString(std::string& output)
{

     std::stringstream file;
     file.precision(2);
     file<<fixed;

     //Point centre (10299,2051);
     Point centre (0,0);
     double factor=100;

     //writing the nodes
     //      int mynodes[] = {47, 30 ,38};
     //      int mynodes[] = {41, 1521};
     int mynodes[] = {0};
     //int mynodes[] = { 28, 27, 40};
     vector<int> nodes_to_plot (mynodes, mynodes + sizeof(mynodes) / sizeof(int) );


     //for (unsigned int n=0;n<new_nodes.size();n++){
     //      JNode* JNode=new_nodes[n];

     for (unsigned int n=0; n<_nodes.size(); n++) {
          JNode* node=_nodes[n];

          int node_id=node->id; //cout<<"node id: "<<node_id<<endl;
          if(nodes_to_plot.size()!=0)
               if (IsElementInVector(nodes_to_plot, node_id) == false)
                    continue;
          //              if(problem_nodes.size()!=0)
          //                      if (IsElementInVector(problem_nodes, node_id) == false)
          //                              continue;

          //if(node->pGroup!="080") continue;
          //              if(node->pPortals.size()<10) continue;
          //if(node->IsConvex()==true) continue;
          //if(node->IsClockwise()==true) continue;

          file<<"\t\t<label centerX=\""<<node->pCentroid.GetX()*factor -centre.GetX()<<"\" centerY=\""<<node->pCentroid.GetY()*factor-centre.GetY()<<"\" centerZ=\"0\" text=\""<<node->id <<"\" color=\"100\" />"<<endl;
          //              cout<<"size: "<< node->pHull.size()<<endl;
          //              std::sort(node->pHull.begin(), node->pHull.end());
          //              node->pHull.erase(std::unique(node->pHull.begin(), node->pHull.end()), node->pHull.end());
          //              cout<<"size: "<< node->pHull.size()<<endl;

          for(unsigned int i=0; i<node->pHull.size(); i++) {
               //double x=pVertices[JNode->pHull[i].id]->pPos.GetX()*factor -centre.pX;
               //double y=pVertices[JNode->pHull[i].id]->pPos.GetY()*factor -centre.pY;
               //file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
               //file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<JNode->pHull[i].id<<"\" color=\"20\" />"<<endl;

               // draw the convex hull
               //                      unsigned int size= node->pHull.size();
               //                      file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
               //                      file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"20\" />"<<endl;
               //                      double x1=pVertices[node->pHull[i%size].id]->pPos.GetX()*factor -centre.pX;
               //                      double y1=pVertices[node->pHull[i%size].id]->pPos.GetY()*factor -centre.pY;
               //                      double x2=pVertices[node->pHull[(i+1)%size].id]->pPos.GetX()*factor -centre.pX;
               //                      double y2=pVertices[node->pHull[(i+1)%size].id]->pPos.GetY()*factor -centre.pY;
               //                      file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
               //                      file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
               //                      file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
               //                      file<<"\t\t</wall>"<<endl;
               //                      cout.precision(2);
               //                      cout<<fixed;
               //                      printf("polygon.push_back(Point_2(%f, %f));\n",x1,y1);
          }
          file<<endl;

          for(unsigned int i=0; i<_obst.size(); i++) {
               JObstacle* obst=_obst[i];

               if(obst->pNode0==node_id ) {
                    double x1=obst->pStart.pPos.GetX()*factor-centre._x;
                    double y1=obst->pStart.pPos.GetY()*factor-centre._y;
                    double x2=obst->pEnd.pPos.GetX()*factor-centre._x;
                    double y2=obst->pEnd.pPos.GetY()*factor-centre._y;

                    //file<<"\t\t<label centerX=\""<<0.5*(x1+x2)<<"\" centerY=\""<<0.5*(y1+y2)<<"\" centerZ=\"0\" text=\""<<obst->id<<"\" color=\"20\" />"<<endl;
                    file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
                    file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
                    file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
                    file<<"\t\t</wall>"<<endl;
               }

          }
          file<<endl;

          for(unsigned int i=0; i<_edges.size(); i++) {
               JEdge* edge=_edges[i];

               if(edge->pNode0==node_id || edge->pNode1==node_id) {
                    double x1=edge->pStart.pPos.GetX()*factor-centre._x;
                    double y1=edge->pStart.pPos.GetY()*factor-centre._y;
                    double x2=edge->pEnd.pPos.GetX()*factor-centre._x;
                    double y2=edge->pEnd.pPos.GetY()*factor-centre._y;

                    file<<"\t\t<label centerX=\""<<0.5*(x1+x2)<<"\" centerY=\""<<0.5*(y1+y2)<<"\" centerZ=\"0\" text=\""<<edge->id<<"\" color=\"20\" />"<<endl;
                    file<<"\t\t<door id = \""<<i<<"\">"<<endl;
                    file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
                    file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
                    file<<"\t\t</door>"<<endl;
               }
          }

          file<<endl;
     }

     //eventually write any goal
     for (map<int, Goal*>::const_iterator itr = _building->GetAllGoals().begin();
               itr != _building->GetAllGoals().end(); ++itr) {

          //int door=itr->first;
          Goal* goal              = itr->second;
          file<<goal->Write()<<endl;
     }
     output=file.str();
}

void NavMesh::WriteToFileTraVisTo(std::string fileName)
{
     ofstream file(fileName.c_str());
     file.precision(2);
     file<<fixed;


     if(file.is_open()==false) {
          Log->Write("\tERROR:\tcould not open the file:  " + fileName +" for writing the mesh");
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
         <<endl;

     //writing the geometry
     string output;
     WriteToString(output);
     file<<"\t<geometry>"<<endl;
     file<<output<<endl;
     file<<"\t</geometry>"<<endl;
     file.close();
}

void NavMesh::WriteToFileTraVisTo(std::string fileName, const std::vector<Point>& points)
{
     ofstream file(fileName.c_str());
     file.precision(2);
     file<<fixed;

     //Point centre (10299,2051);
     Point centre (0,0);
     double factor=100;

     if(file.is_open()==false) {
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



     for(unsigned int i=0; i<points.size(); i++) {

          unsigned int size= points.size();
          double x1=points[ i%size].GetX()*factor -centre._x;
          double y1=points[ i%size].GetY()*factor -centre._y;
          double x2=points[ (i+1)%size].GetX()*factor -centre._x;
          double y2=points[ (i+1)%size].GetY()*factor -centre._y;

          //              draw the convex hull
          file<<" \t\t<sphere centerX=\""<<x1<<"\" centerY=\""<<y1<<"\" centerZ=\"0\" radius=\"150\" color=\"100\" />"<<endl;
          file<<"\t\t<label centerX=\""<<x1<<"\" centerY=\""<<y1<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"20\" />"<<endl;
          file<<"\t\t<label centerX=\""<<0.5*(x1+x2)<<"\" centerY=\""<<0.5*(y1+y2)<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"180\" />"<<endl;
          file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
          file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
          file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
          file<<"\t\t</wall>"<<endl;

          //              cout.precision(2);
          //              cout<<fixed;
          //              printf("polygon.push_back(Point_2(%f, %f));\n",x1,y1);
     }
     file<<endl;


     file<<"\t</geometry>"<<endl;
     file.close();
}

void NavMesh::WriteToFileTraVisTo(std::string fileName, JNode* node)
{

     ofstream file(fileName.c_str());
     file.precision(2);
     file<<fixed;

     //Point centre (10299,2051);
     Point centre (0,0);
     double factor=100;

     if(file.is_open()==false) {
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


     int node_id=node->id; //cout<<"node id: "<<node_id<<endl;

     file<<"\t\t<label centerX=\""<<node->pCentroid.GetX()*factor -centre._x<<"\" centerY=\""<<node->pCentroid.GetY()*factor-centre._y<<"\" centerZ=\"0\" text=\""<<node->id <<"\" color=\"100\" />"<<endl;

     //              cout<<"size: "<< JNode->pHull.size()<<endl;
     //              std::sort(JNode->pHull.begin(), JNode->pHull.end());
     //              JNode->pHull.erase(std::unique(JNode->pHull.begin(), JNode->pHull.end()), JNode->pHull.end());
     //              cout<<"size: "<< JNode->pHull.size()<<endl;

     for(unsigned int i=0; i<node->pHull.size(); i++) {
          //double x=pVertices[JNode->pHull[i].id]->pPos.GetX()*factor -centre.pX;
          //double y=pVertices[JNode->pHull[i].id]->pPos.GetY()*factor -centre.pY;
          //file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
          //file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<JNode->pHull[i].id<<"\" color=\"20\" />"<<endl;

          // draw the convex hull
          //                      unsigned int size= node->pHull.size();
          //                      file<<" \t\t<sphere centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" radius=\"5\" color=\"100\" />"<<endl;
          //                      file<<"\t\t<label centerX=\""<<x<<"\" centerY=\""<<y<<"\" centerZ=\"0\" text=\""<<i<<"\" color=\"20\" />"<<endl;
          //                      double x1=pVertices[node->pHull[i%size].id]->pPos.GetX()*factor -centre.pX;
          //                      double y1=pVertices[node->pHull[i%size].id]->pPos.GetY()*factor -centre.pY;
          //                      double x2=pVertices[node->pHull[(i+1)%size].id]->pPos.GetX()*factor -centre.pX;
          //                      double y2=pVertices[node->pHull[(i+1)%size].id]->pPos.GetY()*factor -centre.pY;
          //                      file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
          //                      file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
          //                      file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
          //                      file<<"\t\t</wall>"<<endl;
          //                      cout.precision(2);
          //                      cout<<fixed;
          //                      printf("polygon.push_back(Point_2(%f, %f));\n",x1,y1);
     }
     file<<endl;

     for(unsigned int i=0; i<_obst.size(); i++) {
          JObstacle* obst=_obst[i];

          if(obst->pNode0==node_id ) {
               double x1=obst->pStart.pPos.GetX()*factor-centre._x;
               double y1=obst->pStart.pPos.GetY()*factor-centre._y;
               double x2=obst->pEnd.pPos.GetX()*factor-centre._x;
               double y2=obst->pEnd.pPos.GetY()*factor-centre._y;

               file<<"\t\t<wall id = \""<<i<<"\">"<<endl;
               file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
               file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
               file<<"\t\t</wall>"<<endl;
          }

     }
     file<<endl;

     for(unsigned int i=0; i<_edges.size(); i++) {
          JEdge* edge=_edges[i];

          if(edge->pNode0==node_id || edge->pNode1==node_id) {
               double x1=edge->pStart.pPos.GetX()*factor-centre._x;
               double y1=edge->pStart.pPos.GetY()*factor-centre._y;
               double x2=edge->pEnd.pPos.GetX()*factor-centre._x;
               double y2=edge->pEnd.pPos.GetY()*factor-centre._y;

               file<<"\t\t<label centerX=\""<<0.5*(x1+x2)<<"\" centerY=\""<<0.5*(y1+y2)<<"\" centerZ=\"0\" text=\""<<edge->id<<"\" color=\"20\" />"<<endl;
               file<<"\t\t<door id = \""<<i<<"\">"<<endl;
               file<<"\t\t\t<point xPos=\""<<x1<<"\" yPos=\""<<y1<<"\"/>"<<endl;
               file<<"\t\t\t<point xPos=\""<<x2<<"\" yPos=\""<<y2<<"\"/>"<<endl;
               file<<"\t\t</door>"<<endl;
          }

     }

     file<<endl;

     file<<"\t</geometry>"<<endl;
     file.close();

}

void NavMesh::WriteToFile(std::string fileName)
{

     ofstream file(fileName.c_str());
     file.precision(2);
     file<<fixed;

     if(file.is_open()==false) {
          cout <<"could not open the file: "<<fileName<<endl;
          return;
     }

     //write the vertices
     //file<<"# vertices section"<<endl;
     file<<_vertices.size()<<endl;
     for (unsigned int v=0; v<_vertices.size(); v++) {
          file<<"\t"<<_vertices[v]->pPos.GetX()<<" " <<_vertices[v]->pPos.GetY()<<endl;

     }

     //write the edges
     //file<<endl<<"# edges section"<<endl;
     file<<_edges.size()<<endl;
     for (unsigned int e=0; e<_edges.size(); e++) {
          //file<<pEdges[e]->pStart.pPos.GetX()<<" " <<pEdges[e]->pStart.pPos.GetY()<<endl;
          //file<<"\t"<<pEdges[e]->pDisp.GetX()<<" " <<pEdges[e]->pDisp.GetY()<<endl;
          file<<"\t";
          file<<_edges[e]->pStart.id<<" " <<_edges[e]->pEnd.id<<" ";
          file<<_edges[e]->pNode0<<" " <<_edges[e]->pNode1<<endl;
     }


     //write the obstacles
     //file<<endl<<"# Obstacles section"<<endl;
     file<<_obst.size()<<endl;
     for (unsigned int ob=0; ob<_obst.size(); ob++) {
          file<<"\t";
          file<<_obst[ob]->pStart.id<<" " <<_obst[ob]->pEnd.id<<" ";
          file<<_obst[ob]->pNode0<<" "<<_obst[ob]->pNextObst<<endl;
     }

     //write the nodes
     //file<<endl<<"# Nodes section"<<endl;

     std::map<string,int> ngroup_to_size;
     for (unsigned int n=0; n<_nodes.size(); n++) {
          ngroup_to_size[_nodes[n]->pGroup]++;
     }

     string previousGroup= _nodes[0]->pGroup;
     file<<endl<<previousGroup<<endl;
     file<<ngroup_to_size[previousGroup]<<"";

     for (unsigned int n=0; n<_nodes.size(); n++) {
          JNode* JNode=_nodes[n];
          string actualGroup=JNode->pGroup;
          if(actualGroup!=previousGroup) {
               previousGroup=actualGroup;
               //file<<"# JNode group"<<endl;
               file<<endl<<previousGroup<<endl;
               file<<ngroup_to_size[previousGroup]<<"";
          }

          //assert(JNode->pObstacles.size()<20);
          //assert(JNode->pPortals.size()<20);
          //file<<"nodeid "<<JNode->id<<endl;
          file<<endl;
          file<<"\t"<<JNode->pCentroid.GetX()<<" "<<JNode->pCentroid.GetY()<<endl;
          file<<"\t"<<JNode->pHull.size()<<" ";
          for(unsigned int i=0; i<JNode->pHull.size(); i++) {
               file<<JNode->pHull[i].id<<" ";
          }
          file<<endl;
          file<<"\t"<<JNode->pNormalVec[0]<<" "<<JNode->pNormalVec[1]<<" "<<JNode->pNormalVec[2]<<endl;


          file<<"\t"<<JNode->pPortals.size()<<" ";
          for(unsigned int i=0; i<JNode->pPortals.size(); i++) {
               file<<JNode->pPortals[i]<<" ";
          }
          file<<endl;

          file<<"\t"<<JNode->pObstacles.size()<<" ";
          for(unsigned int i=0; i<JNode->pObstacles.size(); i++) {
               file<<JNode->pObstacles[i]<<" ";
          }

          file<<endl;
     }

     file.close();
}

int NavMesh::AddVertex(JVertex* v)
{
     for (unsigned int vc = 0; vc < _vertices.size(); vc++) {
          if (_vertices[vc]->pPos.operator ==(v->pPos)) {
#ifdef _DEBUG
               cout << "JVertex already present:" << _vertices[vc]->id << endl;
#endif
               return -1;
          }
     }
     if (_vertices.size() == 0) {
          v->id = 0;
     } else {
          v->id = _vertices[_vertices.size() - 1]->id + 1;
     }
     _vertices.push_back(v);
     return v->id;
}

int NavMesh::AddEdge(JEdge* e)
{
     int id = IsPortal(e->pStart.pPos, e->pEnd.pPos);

     if ((IsElementInVector(_edges, e) == false) && (id == -1)) {
          if (_edges.size() == 0) {
               e->id = 0;
          } else {
               e->id = _edges[_edges.size() - 1]->id + 1;
          }
          _edges.push_back(e);
          return e->id;
     } else {
#ifdef _DEBUG
          cout << "JEdge already present:" << id << endl;
#endif
     }

     return -1;
}

int NavMesh::AddObst(JObstacle* o)
{
     int id= IsObstacle(o->pStart.pPos, o->pEnd.pPos);

     if ( (IsElementInVector(_obst, o) == false) &&
               (id==-1 )) {
          if (_obst.size() == 0) {
               o->id = 0;
          } else {
               o->id = _obst[_obst.size() - 1]->id + 1;
          }
          _obst.push_back(o);
          return o->id;
     } else {
#ifdef _DEBUG
          cout << "Obstacles already present:" << id << endl;
#endif
     }

     return -1;
}

int NavMesh::AddNode(JNode* node)
{

     std::sort(node->pObstacles.begin(),node->pObstacles.end());
     std::sort(node->pPortals.begin(),node->pPortals.end());

     for(unsigned int n=0; n<_nodes.size(); n++) {
          if(*_nodes[n]==*node)  {
#ifdef _DEBUG
               cout << "JNode already present:" << node->id << endl;
#endif
               assert(0);
               return -1;
          }
     }


     if (IsElementInVector(_nodes, node)) {

#ifdef _DEBUG
          cout << "JNode already present:" << node->id << endl;
#endif
          assert(0);
          return -1;
     }

     if (_nodes.size() == 0) {
          node->id = 0;
     } else {
          node->id = _nodes[_nodes.size() - 1]->id + 1;
     }
     _nodes.push_back(node);
     return node->id;
}

NavMesh::JVertex* NavMesh::GetVertex(const Point& p)
{

     for(unsigned int v=0; v<_vertices.size(); v++) {
          if(_vertices[v]->pPos.operator ==(p)) {
               return _vertices[v];
          }
     }
#ifdef _DEBUG
     cout<<"JVertex not found: "<< p.GetX()<<":"<<p.GetY()<<endl;
     cout<<"Adding "<<endl;
     cout<<"pVertices.size()="<<_vertices.size()<<endl;
#endif
     JVertex* v = new JVertex();
     v->pPos= p;
     AddVertex(v);
     //this could lead to an infinite loop if the point cannot be added and cannot be found
     return GetVertex(p);

     //exit(EXIT_FAILURE);
}

int NavMesh::IsPortal(Point& p1, Point& p2)
{

     for(unsigned int i=0; i<_edges.size(); i++) {
          JEdge* e=_edges[i];

          if( (e->pStart.pPos==p1) && (e->pEnd.pPos==p2)) {
               return e->id;
          }

          if( (e->pStart.pPos==p2) && (e->pEnd.pPos==p1)) {
               return e->id;
          }
     }
     return -1;
}


void NavMesh::FinalizeAlphaShape()
{
     Log->Write("INFO:\tFinalizing the mesh with an Alpha Shape");

     //      WriteToFileTraVisTo("arena_envelope.xml",envelope);
     //collect all possible vertices that form that envelope

     vector<Line> envelope;

     Room* outside = _building->GetRoom("outside");
     if(outside==NULL) {
          Log->Write("INFO:\t there is no outside room for constructing the alpha shape");
          exit(EXIT_FAILURE);
     }

     // const Point& env_center=outside->GetSubRoom(0)->GetCentroid();
     // double env_radius= outside->GetSubRoom(0)->GetWall(0).DistTo(env_center);
     //outside->WriteToErrorLog();
     //cout<<"Center:" <<env_center.toString()<<endl;
     //cout<<"Radius:" <<env_radius<<endl; exit(0);


     double xmin=0.1;
     double xmax= 50.0;
     double ymin=0.1;
     double ymax= 44.0;

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* r = _building->GetRoom(i);
          string caption = r->GetCaption();

          //skip the virtual room containing the complete geometry
          if(r->GetCaption()=="outside") continue;
          const Point& centroid0 = Point(0,0);

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k);

               //walls
               const vector<Wall>& walls = s->GetAllWalls();

               for (unsigned w = 0; w < walls.size(); w++) {

                    //FIXME: this method is more general but is not working
                    //if(IsCircleVisibleFromLine(env_center,env_radius,walls[w])==false) continue;

                    Point pt1= walls[w].GetPoint1();
                    Point pt2= walls[w].GetPoint2();

                    if( (xmin < pt1._x) && (pt1._x < xmax) &&
                              (ymin < pt1._y) && (pt1._y < ymax) ) continue;

                    if( (xmin < pt2._x) && (pt2._x < xmax) &&
                              (ymin < pt2._y) && (pt2._y < ymax) ) continue;


                    //first attempt
                    Point P0 = walls[w].GetPoint1();
                    Point P1 = walls[w].GetPoint2();
                    Point D0 = P1 - P0;
                    Point D1 = centroid0-P0;
                    if (D0.Det(D1) < 0) {
                         envelope.push_back(Line(P0, P1));
                    } else {
                         envelope.push_back(Line(P1, P0));
                    }
               }


               const vector<Transition*>& transitions = s->GetAllTransitions();
               for (unsigned t = 0; t < transitions.size(); t++) {

                    //if(IsCircleVisibleFromLine(env_center,env_radius,*transitions[t])==false) continue;

                    Point pt1= transitions[t]->GetPoint1();
                    Point pt2= transitions[t]->GetPoint2();

                    if( (xmin < pt1._x) && (pt1._x < xmax) &&
                              (ymin < pt1._y) && (pt1._y < ymax) ) continue;

                    if( (xmin < pt2._x) && (pt2._x < xmax) &&
                              (ymin < pt2._y) && (pt2._y < ymax) ) continue;

                    //first attempt
                    Point P0 = transitions[t]->GetPoint1();
                    Point P1 = transitions[t]->GetPoint2();
                    Point D0 = P1 - P0;
                    Point D1 = centroid0-P0;
                    if (D0.Det(D1) < 0) {
                         envelope.push_back(Line(P0, P1));
                    } else {
                         envelope.push_back(Line(P1, P0));
                    }
               }
          }
     }

     vector<Point> env;
     for(unsigned int i=0; i<envelope.size(); i++) {
          env.push_back(envelope[i].GetPoint1());
          env.push_back(envelope[i].GetPoint2());
     }

     //WriteToFileTraVisTo("jst_test_ulrich.xml",env);
     //cout<<"done"<<endl;
     //cout<<"env size: "<<envelope.size()<<endl;
     //      exit(0);

     //link those vertices
     vector<Point> Hull;
     Hull.push_back(envelope[envelope.size()-1].GetPoint1());
     Hull.push_back(envelope[envelope.size()-1].GetPoint2());
     envelope.pop_back();

     while(envelope.empty()==false) {
          for(unsigned int i=0; i<envelope.size(); i++) {
               if(envelope[i].GetPoint1()==Hull[Hull.size()-1]) {
                    Hull.push_back(envelope[i].GetPoint2());
                    envelope.erase(envelope.begin()+i);
               } else if(envelope[i].GetPoint2()==Hull[Hull.size()-1]) {
                    Hull.push_back(envelope[i].GetPoint1());
                    envelope.erase(envelope.begin()+i);
               }
          }
     }

     //eject the last point which is a duplicate.
     Hull.pop_back();

     //the surrounding room
     vector<Point> Hull2=_building->GetRoom("outside")->GetSubRoom(0)->GetPolygon();

#ifdef _CGAL
     //print for some check
     //WriteToFileTraVisTo("arena_envelope.xml",Hull);
     //exit(0);
     //now check the polygon with holes.

     //      {
     //              ofstream myfile ("mypoints.pts");
     //              if (myfile.is_open())
     //              {
     //                      //quick testing
     //                      for(unsigned int i=0;i<Hull2.size();i++){
     //                              myfile <<"P "<<Hull2[i].pX <<" "<<Hull2[i].pY<<endl;
     //                      }
     //                      myfile <<"H "<<Hull[0].pX <<" "<<Hull[0].pY<<endl;
     //                      for(unsigned int i=1;i<Hull.size();i++){
     //                              myfile <<"P "<<Hull[i].pX <<" "<<Hull[i].pY<<endl;
     //                      }
     //              }
     //
     //      }
     //WriteToFileTraVisTo("arena_envelope.xml",Hull);

     //perform some tests using CGAL

     //first polygon
     Polygon_2 polygon2;
     Polygon_2 holesP[1];


     for(unsigned int i=0; i<Hull.size(); i++) {
          holesP[0].push_back(Point_2(Hull[i]._x,Hull[i]._y));
     }

     for(unsigned int i=0; i<Hull2.size(); i++) {
          polygon2.push_back(Point_2(Hull2[i]._x,Hull2[i]._y));
     }

     if(holesP[0].is_clockwise_oriented())holesP[0].reverse_orientation();
     if(polygon2.is_clockwise_oriented())polygon2.reverse_orientation();

     assert(holesP[0].is_counterclockwise_oriented());
     assert(polygon2.is_counterclockwise_oriented());
     assert(holesP[0].is_simple());
     assert(polygon2.is_simple());

#endif //_CGAL

     Log->Write("INFO:\tPerforming final triangulation with the outside!");

     DTriangulation* tri= new DTriangulation();
     tri->SetOuterPolygone(Hull);
     tri->AddHole(Hull2);
     tri->Triangulate();
     vector<p2t::Triangle*> triangles=tri->GetTriangles();

     // CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 100, 100, 100));
     // gv.set_line_width(4);
     // gv.set_trace(true);
     // gv.set_bg_color(CGAL::Color(0, 200, 200));
     //              // gv.clear();
     //
     //              // use different colors, and put a few sleeps/clear.
     // gv << CGAL::BLUE;
     // gv.set_wired(true);

     for(unsigned int t=0; t<triangles.size(); t++) {
          p2t::Triangle* tr =triangles[t];

          //      Point_2 P0  = Point_2 (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
          //      Point_2 P1  = Point_2 (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
          //      Point_2 P2  = Point_2 (tr->GetPoint(2)->x,tr->GetPoint(2)->y);
          //      gv << Segment_2(P0,P1);
          //      gv << Segment_2(P1,P2);
          //      gv << Segment_2(P0,P2);

          Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
          Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
          Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

          //create the new nodes

          JNode* new_node = new JNode();
          new_node->pGroup = "outside";
          //to get a correct ID
          AddNode(new_node);
          new_nodes.push_back(new_node);
          new_node->pCentroid= (P0+P1+P2)*(1.0/3);

          new_node->pNormalVec[0]=0.0;
          new_node->pNormalVec[1]=0.0;
          new_node->pNormalVec[2]=0.0;

          // Points are by default counterclockwise
          new_node->pHull.push_back(*(GetVertex(P0)));
          new_node->pHull.push_back(*(GetVertex(P1)));
          new_node->pHull.push_back(*(GetVertex(P2)));

          for (int index=0; index<3; index++) {

               Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
               Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

               int edge_id=IsPortal(P0,P1);
               if(edge_id != -1) {
                    //if(IsElementInVector(new_node->pPortals,edge_id)==false)
                    new_node->pPortals.push_back(edge_id);

                    //invalidate any previous information
                    // they will be set later
                    JEdge* e = _edges[edge_id];
                    e->pNode0=-1;
                    e->pNode1=-1;
               }

               int obstacle_id=IsObstacle(P0,P1);
               if(obstacle_id != -1) {
                    //std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
                    //if(IsElementInVector(new_node->pObstacles,obstacle_id)==false)
                    new_node->pObstacles.push_back(obstacle_id);
                    //pObst[obstacle_id]->pNode0=new_node->id;
               }

               // this portal was newly created
               if ((obstacle_id==-1) && (edge_id==-1)) {

                    JEdge* e= new JEdge();
                    e->pEnd=*GetVertex(P1);
                    e->pStart= *GetVertex(P0);
                    AddEdge(e);

                    //invalidate any previous information
                    // they will be set later
                    e->pNode0=-1;
                    e->pNode1=-1;
                    // caution: the ID is automatically assigned in the AddEdge method
                    //if(IsElementInVector(new_node->pPortals,edge_id)==false)
                    new_node->pPortals.push_back(e->id);

               }
          }
     }

     UpdateEdges();
     delete tri;

     Log->Write("INFO:\t...Done!");

}

void NavMesh::Finalize()
{

     Log->Write("INFO:\tFinalizing the mesh");

     //      WriteToFileTraVisTo("arena_envelope.xml",envelope);
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


     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* r = _building->GetRoom(i);
          string caption = r->GetCaption();

          //skip the virtual room containing the complete geometry
          if(r->GetCaption()=="outside") continue;
          if(r->GetZPos()>6) continue;
          const Point& centroid0 = Point(0,0);

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* s = r->GetSubRoom(k);

               //walls
               const vector<Wall>& walls = s->GetAllWalls();

               for (unsigned w = 0; w < walls.size(); w++) {

                    bool skip=false;
                    for(unsigned int i=0; i<centroids.size(); i++) {
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
                    } else {
                         envelope.push_back(Line(P1, P0));
                    }
               }


               const vector<Transition*>& transitions = s->GetAllTransitions();
               for (unsigned t = 0; t < transitions.size(); t++) {

                    if(transitions[t]->GetSubRoom2() != NULL) continue;

                    bool skip=false;
                    for(unsigned int i=0; i<centroids.size(); i++) {
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
                    } else {
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

     while(envelope.empty()==false) {
          for(unsigned int i=0; i<envelope.size(); i++) {
               if(envelope[i].GetPoint1()==Hull[Hull.size()-1]) {
                    Hull.push_back(envelope[i].GetPoint2());
                    envelope.erase(envelope.begin()+i);
               } else if(envelope[i].GetPoint2()==Hull[Hull.size()-1]) {
                    Hull.push_back(envelope[i].GetPoint1());
                    envelope.erase(envelope.begin()+i);
               }
          }
     }

     //eject the last point which is a duplicate.
     Hull.pop_back();

     //the surrounding room
     vector<Point> Hull2=_building->GetRoom("outside")->GetSubRoom(0)->GetPolygon();

#ifdef _CGAL
     //print for some check
     //WriteToFileTraVisTo("arena_envelope.xml",Hull);
     //exit(0);
     //now check the polygon with holes.

     //      {
     //              ofstream myfile ("mypoints.pts");
     //              if (myfile.is_open())
     //              {
     //                      //quick testing
     //                      for(unsigned int i=0;i<Hull2.size();i++){
     //                              myfile <<"P "<<Hull2[i].pX <<" "<<Hull2[i].pY<<endl;
     //                      }
     //                      myfile <<"H "<<Hull[0].pX <<" "<<Hull[0].pY<<endl;
     //                      for(unsigned int i=1;i<Hull.size();i++){
     //                              myfile <<"P "<<Hull[i].pX <<" "<<Hull[i].pY<<endl;
     //                      }
     //              }
     //
     //      }
     //WriteToFileTraVisTo("arena_envelope.xml",Hull);

     //perform some tests using CGAL

     //first polygon
     Polygon_2 polygon2;
     Polygon_2 holesP[1];


     for(unsigned int i=0; i<Hull.size(); i++) {
          holesP[0].push_back(Point_2(Hull[i]._x,Hull[i]._y));
     }

     for(unsigned int i=0; i<Hull2.size(); i++) {
          polygon2.push_back(Point_2(Hull2[i]._x,Hull2[i]._y));
     }

     if(holesP[0].is_clockwise_oriented())holesP[0].reverse_orientation();
     if(polygon2.is_clockwise_oriented())polygon2.reverse_orientation();

     assert(holesP[0].is_counterclockwise_oriented());
     assert(polygon2.is_counterclockwise_oriented());
     assert(holesP[0].is_simple());
     assert(polygon2.is_simple());

#endif //_CGAL

     Log->Write("INFO:\tPerforming final triangulation with the outside!");

     DTriangulation* tri= new DTriangulation();
     tri->SetOuterPolygone(Hull);
     tri->AddHole(Hull2);
     tri->Triangulate();
     vector<p2t::Triangle*> triangles=tri->GetTriangles();

     // CGAL::Geomview_stream gv(CGAL::Bbox_3(-100, -100, -100, 100, 100, 100));
     // gv.set_line_width(4);
     // gv.set_trace(true);
     // gv.set_bg_color(CGAL::Color(0, 200, 200));
     //              // gv.clear();
     //
     //              // use different colors, and put a few sleeps/clear.
     // gv << CGAL::BLUE;
     // gv.set_wired(true);

     for(unsigned int t=0; t<triangles.size(); t++) {
          p2t::Triangle* tr =triangles[t];

          //      Point_2 P0  = Point_2 (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
          //      Point_2 P1  = Point_2 (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
          //      Point_2 P2  = Point_2 (tr->GetPoint(2)->x,tr->GetPoint(2)->y);
          //      gv << Segment_2(P0,P1);
          //      gv << Segment_2(P1,P2);
          //      gv << Segment_2(P0,P2);

          Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
          Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
          Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

          //create the new nodes

          JNode* new_node = new JNode();
          new_node->pGroup = "outside";
          //to get a correct ID
          AddNode(new_node);
          new_nodes.push_back(new_node);
          new_node->pCentroid= (P0+P1+P2)*(1.0/3);

          new_node->pNormalVec[0]=0.0;
          new_node->pNormalVec[1]=0.0;
          new_node->pNormalVec[2]=0.0;

          // Points are by default counterclockwise
          new_node->pHull.push_back(*(GetVertex(P0)));
          new_node->pHull.push_back(*(GetVertex(P1)));
          new_node->pHull.push_back(*(GetVertex(P2)));

          for (int index=0; index<3; index++) {

               Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
               Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

               int edge_id=IsPortal(P0,P1);
               if(edge_id != -1) {
                    //if(IsElementInVector(new_node->pPortals,edge_id)==false)
                    new_node->pPortals.push_back(edge_id);

                    //invalidate any previous information
                    // they will be set later
                    JEdge* e = _edges[edge_id];
                    e->pNode0=-1;
                    e->pNode1=-1;
               }

               int obstacle_id=IsObstacle(P0,P1);
               if(obstacle_id != -1) {
                    //std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
                    //if(IsElementInVector(new_node->pObstacles,obstacle_id)==false)
                    new_node->pObstacles.push_back(obstacle_id);
                    //pObst[obstacle_id]->pNode0=new_node->id;
               }

               // this portal was newly created
               if ((obstacle_id==-1) && (edge_id==-1)) {

                    JEdge* e= new JEdge();
                    e->pEnd=*GetVertex(P1);
                    e->pStart= *GetVertex(P0);
                    AddEdge(e);

                    //invalidate any previous information
                    // they will be set later
                    e->pNode0=-1;
                    e->pNode1=-1;
                    // caution: the ID is automatically assigned in the AddEdge method
                    //if(IsElementInVector(new_node->pPortals,edge_id)==false)
                    new_node->pPortals.push_back(e->id);

               }
          }
     }

     UpdateEdges();
     delete tri;

     Log->Write("INFO:\t...Done!");
}


void NavMesh::Triangulate(SubRoom* sub)
{

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

     for(unsigned int p=0; p<obstacles.size(); p++) {
          holes.push_back(obstacles[p]->GetPolygon());
     }


#ifdef _CGAL

     //perform some checks
     Polygon_2 polygon;
     Polygon_2 holesP[holes.size()];

     for(unsigned int i=0; i<outerHull.size(); i++) {
          polygon.push_back(Point_2(outerHull[i]._x,outerHull[i]._y));
     }
     assert(polygon.is_simple());
     if(polygon.is_clockwise_oriented()) {
          std::reverse(outerHull.begin(), outerHull.end());
          polygon.reverse_orientation();
     }
     assert(polygon.is_counterclockwise_oriented());


     for(unsigned int i=0; i<holes.size(); i++) {
          for(unsigned int j=0; j<holes[i].size(); j++) {
               holesP[i].push_back(Point_2(holes[j][i]._x,holes[j][i]._y));
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
     for(unsigned int i=0; i<holes.size(); i++) {
          tri->AddHole(holes[i]);
     }

     tri->Triangulate();

     vector<p2t::Triangle*> triangles=tri->GetTriangles();

     for(unsigned int t=0; t<triangles.size(); t++) {
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
          new_node->pCentroid= (P0+P1+P2)*(1.0/3);

          new_node->pNormalVec[0]=0;
          new_node->pNormalVec[1]=0;
          new_node->pNormalVec[2]=_building->GetRoom(sub->GetRoomID())->GetZPos();

          // Points are by default counterclockwise
          new_node->pHull.push_back(*(GetVertex(P0)));
          new_node->pHull.push_back(*(GetVertex(P1)));
          new_node->pHull.push_back(*(GetVertex(P2)));


          for (int index=0; index<3; index++) {

               Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
               Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

               int edge_id=IsPortal(P0,P1);
               if(edge_id != -1) {
                    new_node->pPortals.push_back(edge_id);
                    JEdge* e = _edges[edge_id];

                    // invalidate the node
                    e->pNode0=-1;
                    e->pNode1=-1;

               }

               int obstacle_id=IsObstacle(P0,P1);
               if(obstacle_id != -1) {
                    //std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
                    new_node->pObstacles.push_back(obstacle_id);
                    _obst[obstacle_id]->pNode0=new_node->id;
               }

               // this portal was newly created
               if ((obstacle_id==-1) && (edge_id==-1)) {

                    JEdge* e= new JEdge();
                    e->pEnd=*GetVertex(P1);
                    e->pStart= *GetVertex(P0);
                    AddEdge(e);

                    // invalidate the node
                    e->pNode0=-1;
                    e->pNode1=-1;

                    // caution: the ID is automatically assigned in the AddEdge method
                    new_node->pPortals.push_back(e->id);
               }
          }
     }

     UpdateEdges();
}

void NavMesh::Triangulate(JNode* node)
{

     //if(node->IsConvex()) return;

     vector<Point> outerHull;

     for(unsigned int h=0; h<node->pHull.size(); h++) {
          outerHull.push_back(node->pHull[h].pPos);
     }

     DTriangulation* tri= new DTriangulation();
     tri->SetOuterPolygone(outerHull);

     //treating obstacles as holes
     int rID=map_node_to_subroom[node->id].first;
     int sID=map_node_to_subroom[node->id].second;
     SubRoom* sub = _building->GetRoom(rID)->GetSubRoom(sID);

     const vector<Obstacle*> obstacles=sub->GetAllObstacles();

     for( unsigned int a = 0; a < obstacles.size(); a++) {

          Obstacle* obst=  obstacles[a];

          const vector<Point>& pol =obst->GetPolygon();
          // Vertices
          for (unsigned int p = 0; p < pol.size(); p++) {
               JVertex* v = new JVertex();
               v->pPos= pol[p];
               if(AddVertex(v)==-1) {
                    delete v;
               }
          }

          //obstacles
          const vector<Wall>& walls = obst->GetAllWalls();
          for (unsigned w = 0; w < walls.size(); w++) {
               const Point& centroid0 = obst->GetCentroid();
               //int node0 = s->GetUID();

               JObstacle* o= new JObstacle();
               o->pNode0=node->id;
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

               } else {
                    o->pStart= *GetVertex(P1);
                    //o->pDisp=Point(0,0)-D0;
                    o->pEnd=*GetVertex(P0);
               }

               if (AddObst(o)==-1) {
                    // the JEdge is already there
                    o->id=IsObstacle(o->pStart.pPos, o->pEnd.pPos);
                    delete o;
               }
               //node->pObstacles.push_back(o->id);
          }
          tri->AddHole(pol);
          //cout<<"obstacle in sub"<<endl;
          //exit(0);
     }

     //tri->AddHole(Hull2);
     tri->Triangulate();
     vector<p2t::Triangle*> triangles=tri->GetTriangles();

     for(unsigned int t=0; t<triangles.size(); t++) {
          p2t::Triangle* tr =triangles[t];

          Point P0  = Point (tr->GetPoint(0)->x,tr->GetPoint(0)->y);
          Point P1  = Point (tr->GetPoint(1)->x,tr->GetPoint(1)->y);
          Point P2  = Point (tr->GetPoint(2)->x,tr->GetPoint(2)->y);

          //create the new nodes
          JNode* new_node = new JNode();
          new_node->pGroup = node->pGroup;
          //to get a correct ID
          AddNode(new_node);

          assert(new_node->id!=-1);

          new_nodes.push_back(new_node);
          new_node->pCentroid= (P0+P1+P2)*(1.0/3);

          new_node->pNormalVec[0]=node->pNormalVec[0];
          new_node->pNormalVec[1]=node->pNormalVec[1];
          new_node->pNormalVec[2]=node->pNormalVec[2];

          // Points are by default counterclockwise
          new_node->pHull.push_back(*(GetVertex(P0)));
          new_node->pHull.push_back(*(GetVertex(P1)));
          new_node->pHull.push_back(*(GetVertex(P2)));


          for (int index=0; index<3; index++) {

               Point P0  = Point (tr->GetPoint(index%3)->x,tr->GetPoint(index%3)->y);
               Point P1  = Point (tr->GetPoint((index+1)%3)->x,tr->GetPoint((index+1)%3)->y);

               int edge_id=IsPortal(P0,P1);
               if(edge_id != -1) {
                    new_node->pPortals.push_back(edge_id);
                    JEdge* e = _edges[edge_id];

                    //invalidate the node
                    e->pNode0=-1;
                    e->pNode1=-1;
               }

               int obstacle_id=IsObstacle(P0,P1);
               if(obstacle_id != -1) {
                    //std::cerr<<"Error: the convexification has created an JObstacle"<<endl;
                    new_node->pObstacles.push_back(obstacle_id);
                    //                              pObst[obstacle_id]->pNode0=new_node->id;
                    _obst[obstacle_id]->pNode0=-1;
               }

               // this portal was newly created
               if ((obstacle_id==-1) && (edge_id==-1)) {

                    JEdge* e= new JEdge();
                    e->pEnd=*GetVertex(P1);
                    e->pStart= *GetVertex(P0);
                    AddEdge(e);

                    //invalidate the node
                    e->pNode0=-1;
                    e->pNode1=-1;

                    // caution: the ID is automatically assigned in the AddEdge method
                    new_node->pPortals.push_back(e->id);
               }
          }
     }

     //return; //fixme
     {
          // now post processing the newly created nodes
          assert ((unsigned int)node->id != (_nodes.size() -1) && "Trying to remove the last node !");
          JNode* new_node = _nodes.back();
          _nodes.pop_back();

          //making the transformation

          for(unsigned int i=0; i<new_node->pObstacles.size(); i++) {
               _obst[new_node->pObstacles[i]]->pNode0=node->id;
          }


          for(unsigned int i=0; i<new_node->pPortals.size(); i++) {

               if(_edges[new_node->pPortals[i]]->pNode0==new_node->id) {
                    _edges[new_node->pPortals[i]]->pNode0=node->id;
               } else {
                    _edges[new_node->pPortals[i]]->pNode1=node->id;
               }
          }

          new_node->id=node->id;
          _nodes[node->id]=new_node;

          delete node;
     }

     UpdateEdges();
     UpdateObstacles();

     delete tri;
}

int NavMesh::IsObstacle(Point& p1, Point& p2)
{

     for(unsigned int i=0; i<_obst.size(); i++) {
          JObstacle* obst=_obst[i];

          if( (obst->pStart.pPos==p1) && (obst->pEnd.pPos==p2)) {
               return obst->id;
          }

          if( (obst->pStart.pPos==p2) && (obst->pEnd.pPos==p1)) {
               return obst->id;
          }
     }

     return -1;
}

void NavMesh::WriteScenario()
{
     WriteBehavior();
     WriteViewer();
     WriteStartPositions();
}

void NavMesh::WriteBehavior()
{
     string filename="../pedunc/examples/stadium/arenaB.xml";
     ofstream file(filename.c_str());
     file.precision(2);
     file<<fixed;

     if(file.is_open()==false) {
          cout <<"could not open the file: "<<filename<<endl;
          return;
     }

     file<< "<?xml version=\"1.0\"?>"<<endl;
     file<< "\t<Population>"<<endl;


     int goalsetid=0;
     //Write the goal set outside
     {
          file<< "\t\t<GoalSet id=\""<<goalsetid++<<"\" description=\"outside\">"<<endl;

          vector<Point> goals=_building->GetRoom("outside")->GetSubRoom(0)->GetPolygon();
          for(unsigned int g=0; g<goals.size(); g++) {
               double factor=(10.0/(goals[g].Norm())-1);
               file<< "\t\t\t<Goal type=\"point\" id=\""<<g<<"\" x=\""<< factor*goals[g]._x<<"\" y=\""<<factor*goals[g]._y<<"\"/>"<<endl;
          }
          file<< "\t\t</GoalSet>"<<endl;
     }


     //write the goal set tunnel
     {
          file<< "\t\t<GoalSet id=\""<<goalsetid++<<"\" description=\"tunnel\">"<<endl;

          for (map<int, Transition*>::const_iterator itr = _building->GetAllTransitions().begin();
                    itr != _building->GetAllTransitions().end(); ++itr) {

               int door=itr->first;
               //int door = itr->second->GetUniqueID();
               Transition* cross = itr->second;
               const Point& centre = cross->GetCentre();

               if((cross->Length()<2.6) && (cross->Length()>2.4))

                    file<< "\t\t\t<Goal type=\"point\" id=\""<<door<<"\" x=\""<< centre._x<<"\" y=\""<<centre._y<<"\"/>"<<endl;
          }
          file<< "\t\t</GoalSet>"<<endl;
     }

     //write the goal set promenade

     file<< "\t\t<Behavior class=\"1\">"<<endl;
     file<< "\t\t\t<Property name=\"prefSpeed\" type=\"float\" dist=\"c\" value=\"1.3\" />"<<endl;
     file<< "\t\t\t<Property type=\"2D\" name=\"stride\" dist=\"c\" factor_value=\"100.57\" buffer_value=\"0.0\" />"<<endl;
     file<< ""<<endl;
     file<< "\t\t\t<State name=\"Walk1\" speedPolicy=\"min\" final=\"0\" >"<<endl;
     file<< "<!--"<<endl;
     file<< "<NavMeshGoal goalSet=\"0\" goal=\"farthest\" filename=\"../examples/stadium/arena.nav\"/>"<<endl;
     file<< "-->"<<endl;
     file<< "\t\t\t\t<AbsoluteGoal goalSet=\"0\" goal=\"nearest\" perAgent=\"1\" />"<<endl;
     file<< "\t\t\t\t<VelComponent type=\"navMesh\" weight=\"1.0\"  filename=\"../examples/stadium/arena.nav\" />"<<endl;
     file<< "\t\t\t</State>"<<endl;
     file<< "\t\t\t<State name=\"Stop1\" speedPolicy=\"min\" final=\"1\">"<<endl;
     file<< "\t\t\t\t<HoldPosGoal />"<<endl;
     file<< "\t\t\t\t<VelComponent type=\"goal\" weight=\"1.0\"/>"<<endl;
     file<< "\t\t\t</State>"<<endl;
     file<< ""<<endl;
     file<< "\t\t\t<Transition order=\"0\" type=\"goal_circle\" from=\"Walk1\" to=\"Stop1\" radius=\"0.25\" inside=\"1\" />"<<endl;
     file<< "\t\t</Behavior>"<<endl;
     file<< ""<<endl;
     file<< "</Population>"<<endl;


     file.close();
}

void NavMesh::WriteViewer()
{
     /*
     <?xml version="1.0"?>

     <View width="640" height="480">
         <!-- Multiple cameras are mapped to a key from 1-9 in the order they are defined here -->
             <Camera xpos="6.53453" ypos="7.05969" zpos="-4.31638" xtgt="0.609475" ytgt="0.961173" ztgt="1.77459" far="200" near="0.01" fov="45" />
             <Camera xpos="-0.947526" ypos="17.2771" zpos="1.64757" xtgt="-0.947526" ytgt="2.61554" ztgt="1.64584" far="200" near="0.01" fov="0.0" />

             <!-- Comment out lights for a constant-illuminated visualization -->
             <Light x="1" y="0" z="-1" type="directional" diffR="1.0" diffG="0.8" diffB="0.8" space="camera"/>
             <Light x="-1" y="0" z="-1" type="directional" diffR="0.8" diffG="0.8" diffB="1.0" space="camera"/>
             <Light x="0" y="1" z="0" type="directional" diffR="0.8" diffG="0.8" diffB="0.8" space="world"/>
     </View>
      */

}

void NavMesh::WriteStartPositions()
{

     //get the available positions:

     PedDistributor* pDistribution = new PedDistributor();

     vector< vector<Point > >  availablePos = vector< vector<Point> >();

     for (int r = 0; r < _building->GetNumberOfRooms(); r++) {
          vector<Point >   freePosRoom =  vector<Point >  ();
          Room* room = _building->GetRoom(r);
          if(room->GetCaption()=="outside") continue;
          for (int s = 0; s < room->GetNumberOfSubRooms(); s++) {
               SubRoom* subr = room->GetSubRoom(s);
               vector<Point > pos = pDistribution->PossiblePositions(subr);
               freePosRoom.insert(freePosRoom.end(),pos.begin(),pos.end());
          }
          availablePos.push_back(freePosRoom);
     }


     string filename="../pedunc/examples/stadium/arenaS.xml";
     ofstream file(filename.c_str());
     file.precision(2);
     file<<fixed;

     if(file.is_open()==false) {
          cout <<"could not open the file: "<<filename<<endl;
          return;
     }

     file<< "<?xml version=\"1.0\"?>"<<endl;
     file<< "<Experiment version=\"2.0\">"<<endl;
     file<< "\t<SpatialQuery>"<<endl;
     file<< "\t\t<NavMesh filename=\"../examples/stadium/arena.nav\"/>"<<endl;
     file<< "\t</SpatialQuery>"<<endl;
     file<< "<!--"<<endl;
     file<< "<Elevation>"<<endl;
     file<< "<NavMeshElevation filename=\"../examples/stadium/arena.nav\" />"<<endl;
     file<< "</Elevation>"<<endl;
     file<< "-->"<<endl;
     file<< "\t<Boids max_force=\"8\" leak_through=\"0.1\" reaction_time=\"0.5\" />"<<endl;
     file<< "\t<Common time_step=\"0.1\" />"<<endl;
     file<< "\t<GCF reaction_time=\"0.5\" max_agent_dist=\"2\" max_agent_force=\"3\" agent_interp_width=\"0.1\" nu_agent=\"0.35\" />"<<endl;
     file<< "\t<Helbing agent_scale=\"2000\" obstacle_scale=\"4000\" reaction_time=\"0.5\" body_force=\"1200\" friction=\"2400\" force_distance=\"0.015\" />"<<endl;
     file<< "\t<Johansson agent_scale=\"25\" obstacle_scale=\"35\" reaction_time=\"0.5\" force_distance=\"0.15\" stride_time=\"0.5\" />"<<endl;
     file<< "\t<Karamouzas orient_weight=\"0.8\" fov=\"200\" reaction_time=\"0.4\" wall_steepness=\"2\" wall_distance=\"2\" colliding_count=\"5\" d_min=\"1\" d_mid=\"8\" d_max=\"10\" agent_force=\"4\" />"<<endl;
     file<< "\t<Zanlungo agent_scale=\"2000\" obstacle_scale=\"4000\" reaction_time=\"0.5\" force_distance=\"0.005\" />"<<endl;
     file<< ""<<endl;
     file<< "\t<AgentSet>"<<endl;
     file<< "\t\t<Boids tau=\"3\" tauObst=\"6\" />"<<endl;
     file<< "\t\t<Common max_neighbors=\"10\" obstacleSet=\"1\" neighbor_dist=\"5\" r=\"0.19\" class=\"1\" pref_speed=\"1.04\" max_speed=\"2\" max_accel=\"5\" />"<<endl;
     file<< "\t\t<GCF stand_depth=\"0.18\" move_scale=\"0.53\" slow_width=\"0.25\" sway_change=\"0.05\" orient_weight=\"0.75\" />"<<endl;
     file<< "\t\t<Helbing mass=\"80\" />"<<endl;
     file<< "\t\t<Johansson fov_weight=\"0.16\" />"<<endl;
     file<< "\t\t<Karamouzas personal_space=\"0.69\" anticipation=\"8\" />"<<endl;
     file<< "\t\t<RVO tau=\"3\" tauObst=\"0.75\" turningBias=\"1.0\" />"<<endl;
     file<< "\t\t<Zanlungo mass=\"80\" orient_weight=\"0.75\" />"<<endl;
     file<< ""<<endl;


     for(int i=0; i<_building->GetNumberOfRooms(); i++) {
          //int room_id=pBuilding->GetRoom("100")->GetRoomID();

          Room* room = _building->GetRoom(i);
          if(room->GetCaption()=="outside") continue;
          if(room->GetCaption()=="150") continue;
          int room_id=room->GetID();
          vector<Point > freePosRoom = availablePos[room_id];

          int nAgentsPerRoom=10; // the number of agents to distribute
          for (int a=0; a<nAgentsPerRoom; a++) {
               int index = rand() % freePosRoom.size();
               file<< "\t\t<Agent p_x=\""<<freePosRoom[index]._x<<" \"p_y=\""<<freePosRoom[index]._y<<"\"/>"<<endl;
               //cout<<"Position: "<<freePosRoom[index].toString()<<endl;
               freePosRoom.erase(freePosRoom.begin() + index);
          }

          //              break;
     }

     file<< "\t</AgentSet>"<<endl;
     file<< "</Experiment>"<<endl;

}

void NavMesh::UpdateEdges()
{

     for(unsigned int n=0; n<_nodes.size(); n++) {
          JNode* node= _nodes[n];

          std::sort( node->pPortals.begin(), node->pPortals.end() );
          node->pPortals.erase( std::unique( node->pPortals.begin(), node->pPortals.end() ), node->pPortals.end() );

          for(unsigned int i=0; i<node->pPortals.size(); i++) {
               JEdge* e= _edges[node->pPortals[i]];
               if(e->pNode0<0 && e->pNode1!=node->id) {
                    e->pNode0=node->id;
               } else if(e->pNode1<0 && e->pNode0!=node->id) {
                    e->pNode1=node->id;
               }

               if(e->pNode0>e->pNode1) {
                    swap(e->pNode0,e->pNode1);
               }
               if((e->pNode0==e->pNode1) && (e->pNode1!=-1)) {
                    cout<<"Duplicate: "<<endl;
                    cout<<"edge id: "<< e->id <<endl;
                    cout<<"node 0 : "<< e->pNode0 <<endl;
                    cout<<"node 1 : "<< e->pNode1 <<endl;
                    exit(0);
               }
          }
     }
}

void NavMesh::UpdateObstacles()
{

     for(unsigned int n=0; n<_nodes.size(); n++) {
          JNode* node= _nodes[n];
          for(unsigned int i=0; i<node->pObstacles.size(); i++) {
               JObstacle* o= _obst[node->pObstacles[i]];

               if(o->pNode0<0) {
                    o->pNode0=node->id;
               }
          }
     }
}

//void NavMesh::ComputePlaneEquation(SubRoom* sub, double* coefficents) {
//
//      double StairAngle=34.0; // degrees
//      double theta = ( StairAngle * M_PI / 180.0 );
//      double StairAreaToIgnore=5.0;
//
//      Room* room=pBuilding->GetRoom(sub->GetRoomID());
//      coefficents[0]=0;
//      coefficents[1]=0;
//      coefficents[2]=room->GetZPos(); //default elevation
//
//      Stair* stair=dynamic_cast<Stair*>(sub);
//      if(stair==NULL)
//      {
////
////            if ((sub->GetAllCrossings().size())
////                            + sub->GetAllTransitions().size() >2)
////                    return;
//
//              Point projection;
//              bool connection=false;
//
//              //check if the subroom is connected with a stair
//              for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
//                      Room* r = pBuilding->GetRoom(i);
//                      for (int k = 0; k < r->GetAnzSubRooms(); k++) {
//                              SubRoom* s = r->GetSubRoom(k);
//                              Stair* st=dynamic_cast<Stair*>(s);
//                              if ((st!=NULL) && (s->GetSubRoomID()!=sub->GetSubRoomID()) ){
//                                      if(st->GetAllCrossings().size()==2) continue;
//                                      if(sub->IsDirectlyConnectedWith(s)){
//                                              //get the middle point of the crossing
//                                              //check the crossings
//                                              const vector<Crossing*>& crossings1 = sub->GetAllCrossings();
//                                              const vector<Crossing*>& crossings2 = s->GetAllCrossings();
//                                              for (unsigned int c1 = 0; c1 < crossings1.size(); c1++) {
//                                                      for (unsigned int c2 = 0; c2 < crossings2.size(); c2++) {
//                                                              int uid1 = crossings1[c1]->GetUniqueID();
//                                                              int uid2 = crossings2[c2]->GetUniqueID();
//                                                              // ignore my transition
//                                                              if (uid1 == uid2){
//                                                                      Line axe(st->GetUp(), st->GetDown());
//                                                              //      projection=axe.ShortestPoint(crossings1[c1]->GetCentre());
//                                                                      projection=crossings1[c1]->GetCentre();
//                                                                      connection=true;
//                                                                      goto DONE;
//                                                              }
//                                                      }
//                                              }
//                                      }
//                              }
//
//                      }
//              }
//              // do the projection
//              DONE:
//              if(connection){
//                      coefficents[2]=room->GetZPos()*0 + projection.Norm()*tan(theta);
//              }
//
//      }
//      else
//      { // we are having a stair
//              //return;
////            cout<<"area: " <<stair->GetArea()<<endl;
////            if(stair->GetArea()<StairAreaToIgnore)  {
////                    return;
////            }
////            if(stair->GetAllCrossings().size()==2) return;
//
//              // looking for the normal vector
//              Point A;
//              Point B;
//              Point C;
//              Point D;
//              bool finished=false;
//              const vector<Point>& poly=sub->GetPolygon();
//              //loop until we get something
//              while ( ! finished) {
//
//                      for (unsigned int i1=0;i1<4;i1++){
//
//                              int i2 = (i1 + 1) % poly.size();
//                              int i3 = (i2 + 1) % poly.size();
//                              int i4 = (i3 + 1) % poly.size();
//                              Point p1 = poly[i1];
//                              Point p2 = poly[i2];
//                              Point p3 = poly[i3];
//                              Point p4 = poly[i4];
//                              if( (p1-p2).Norm() < (p3-p2).Norm() ){
//                                      //take the closest to the center of the stadium
//                                      double dist1= Line(p1,p2).DistTo(Point(0,0));
//                                      double dist2= Line(p3,p4).DistTo(Point(0,0));
//                                      if(dist1<dist2){
//                                              Point D0 = p1 - p2;
////                                            Point D1 = Point(0,0)-p1;
//                                              Point D1 = p1 - Point(0,0);
//                                              if (D0.Det(D1) > 0) {
//                                                      D=p1;
//                                                      A=p2;
//                                                      B=p3;
//                                                      C=p4;
//                                              }else {
//                                                      A=p1;
//                                                      B=p4;
//                                                      C=p3;
//                                                      D=p2;
//                                              }
//                                              finished=true;
//                                      }else {cout<<"dist: " <<dist2<<endl;}
//                              }
//                      }
//              }
//
//              double base=room->GetZPos();
//
//              double vecDA[3];
//              vecDA[0]= (A-D).pX;
//              vecDA[1]= (A-D).pY;
//              vecDA[2]= 0.0;
//
//              double vecDC[3];
//              vecDC[0]= (C-D).pX;
//              vecDC[1]= (C-D).pY;
//              vecDC[2]= (C-D).Norm()*cos(theta);
//
//              double vecNormal[3];
//              vecNormal[0]= vecDA[1]*vecDC[2] - vecDA[2]*vecDC[1];
//              vecNormal[1]= vecDA[2]*vecDC[0] - vecDA[0]*vecDC[2];
//              vecNormal[2]= vecDA[0]*vecDC[1] - vecDA[1]*vecDC[0];
//
//
//              // the equation of the plan is given as: Ax+By+Cz+d=0;
//              // using the Point A:
//              double d = - (vecNormal[0]*A.pX+vecNormal[1]*A.pY+vecNormal[2]*base);
//              coefficents[0]= - vecNormal[0] / vecNormal[2];
//              coefficents[1]= - vecNormal[1] / vecNormal[2];
//              coefficents[2]= - d / vecNormal[2];
//      }
//
//}

void NavMesh::ComputePlanesEquation()
{

     //first compute the stairs equations.
     // all other equations are derived from there.

     ComputeStairsEquation();

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* r = _building->GetRoom(i);
          //if(r->GetCaption()!="090") continue;

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* sub = r->GetSubRoom(k);

               Stair* stair=dynamic_cast<Stair*>(sub);

               if(stair==NULL) {
                    bool connection=false;

                    //check if the subroom is connected with a stair
                    for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
                         Room* r = _building->GetRoom(i);
                         for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
                              SubRoom* s = r->GetSubRoom(k);
                              Stair* st=dynamic_cast<Stair*>(s);
                              //if ((st!=NULL) && (s->GetSubRoomID()!=sub->GetSubRoomID()) ){
                              if (st!=NULL) {
                                   //if(st->GetAllCrossings().size()==2) continue;
                                   if(sub->IsDirectlyConnectedWith(st)) {
                                        //get the middle point of the crossing
                                        //check the crossings
                                        const vector<Crossing*>& crossings1 = sub->GetAllCrossings();
                                        const vector<Crossing*>& crossings2 = st->GetAllCrossings();
                                        for (unsigned int c1 = 0; c1 < crossings1.size(); c1++) {
                                             for (unsigned int c2 = 0; c2 < crossings2.size(); c2++) {
                                                  int uid1 = crossings1[c1]->GetUniqueID();
                                                  int uid2 = crossings2[c2]->GetUniqueID();
                                                  // ignore my transition
                                                  if (uid1 == uid2) {
                                                       Point center=crossings1[c1]->GetCentre();
                                                       double elevation = st->GetElevation(center);
                                                       sub->SetPlanEquation(0.0,0.0,elevation);
                                                       connection=true;
                                                       goto DONE; // just out of this ugly loop
                                                  }
                                             }
                                        }
                                        const vector<Transition*>& transitions1 = sub->GetAllTransitions();
                                        const vector<Transition*>& transitions2 = st->GetAllTransitions();
                                        for (unsigned int t1 = 0; t1 < transitions1.size(); t1++) {
                                             for (unsigned int t2 = 0; t2 < transitions2.size(); t2++) {
                                                  int uid1 = transitions1[t1]->GetUniqueID();
                                                  int uid2 = transitions2[t2]->GetUniqueID();
                                                  // ignore my transition
                                                  if (uid1 == uid2) {
                                                       Point center=transitions1[t1]->GetCentre();
                                                       double elevation = st->GetElevation(center);
                                                       sub->SetPlanEquation(0.0,0.0,elevation);
                                                       connection=true;
                                                       goto DONE; // just out of this ugly loop
                                                  }
                                             }
                                        }
                                   }
                              }
                         }
                    }
                    // do the projection
DONE:
                    if(connection==false) {
                         sub->SetPlanEquation(0.0,0.0,r->GetZPos());
                         //cout<<"base: "<< sub->GetAllCrossings().size()<<endl;
                    }
               }
          }
     }
}

void NavMesh::UpdateNodes()
{
     //loop over the nodes
     //loop over the obstacles and connect the obstacles which
     //share an Obstacle.

     for (unsigned int i = 0; i < _nodes.size(); i++) {
          JNode* node = _nodes[i];
          //node->pObstacles.clear();

          for (unsigned int j = 0; j < node->pHull.size(); j++) {
               const Point& V = _vertices[node->pHull[j].id]->pPos;
               for (unsigned int k = 0; k < _obst.size(); k++) {
                    const Point& A = _obst[k]->pEnd.pPos;
                    const Point& B = _obst[k]->pStart.pPos;
                    if ( (A==V) || (B==V) ) node->pObstacles.push_back(_obst[k]->id);
               }
          }
          std::sort( node->pObstacles.begin(), node->pObstacles.end() );
          node->pObstacles.erase( std::unique( node->pObstacles.begin(), node->pObstacles.end() ), node->pObstacles.end() );
     }

}

void NavMesh::ComputeStairsEquation()
{

     double StairAngle=34.0; // degrees
     double theta = ( StairAngle * M_PI / 180.0 );

     for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
          Room* r = _building->GetRoom(i);
          //cout<<"room: "<<r->GetCaption()<<endl;
          //cout<<"elevation: "<<base<<endl;

          //if(r->GetCaption()!="090") continue;

          for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
               SubRoom* sub = r->GetSubRoom(k);

               Stair* stair=dynamic_cast<Stair*>(sub);
               double base=r->GetZPos();


               if(stair!=NULL) {
                    // we are having a stair
                    //return;
                    //              cout<<"area: " <<stair->GetArea()<<endl;
                    //              if(stair->GetArea()<StairAreaToIgnore)  {
                    //                      return;
                    //              }

                    if(stair->GetAllCrossings().size()<=4) {
                         stair->SetPlanEquation(0.0,0.0,r->GetZPos());
                         //cout<<"elevation: " <<base<<endl;
                         //getc(stdin);
                         continue;
                    }

                    // looking for the normal vector
                    Point A;
                    Point B;
                    Point C;
                    Point D;
                    bool finished=false;
                    vector<Point> poly=sub->GetPolygon();
                    {
                         Point vecAB= poly[1]-poly[0];
                         Point vecBC= poly[2]-poly[1];

                         double det=vecAB.Det(vecBC);
                         if(fabs(det)>J_EPS) {
                              std::reverse(poly.begin(), poly.end());
                              //cout<<"stair is ccw:"<<endl;
                         }
                    }

                    //loop until we get something
                    while ( ! finished) {

                         for (unsigned int i1=0; i1<4; i1++) {

                              int i2 = (i1 + 1) % poly.size();
                              int i3 = (i2 + 1) % poly.size();
                              int i4 = (i3 + 1) % poly.size();
                              Point p1 = poly[i1];
                              Point p2 = poly[i2];
                              Point p3 = poly[i3];
                              Point p4 = poly[i4];
                              if( (p1-p2).Norm() < (p3-p2).Norm() ) {
                                   //take the closest to the center of the stadium
                                   double dist1= Line(p1,p2).DistTo(Point(0.0,0.0));
                                   double dist2= Line(p3,p4).DistTo(Point(0.0,0.0));
                                   if(dist1<dist2) {
                                        Point D0 = p2 - p1;
                                        Point D1 = Point(0.0,0.0)-p1;
                                        //Point D1 = p1 - Point(0,0);
                                        if (D0.Det(D1) > 0) {
                                             D=p1;
                                             A=p2;
                                             B=p3;
                                             C=p4;
                                             //finished=true;
                                        } else {
                                             A=p1;
                                             B=p4;
                                             C=p3;
                                             D=p2;
                                        }
                                        finished=true;
                                   }
                                   //                                                      if(dist1<dist2){
                                   //                                                              Point D0 = p1 - p2;
                                   //                                                              //Point D1 = Point(0,0)-p1;
                                   //                                                              Point D1 = p1 - Point(0,0);
                                   //                                                              if (D0.Det(D1) > 0) {
                                   //                                                                      D=p1;
                                   //                                                                      A=p2;
                                   //                                                                      B=p3;
                                   //                                                                      C=p4;
                                   //                                                                      //finished=true;
                                   //                                                              }else {
                                   //                                                                      A=p1;
                                   //                                                                      B=p4;
                                   //                                                                      C=p3;
                                   //                                                                      D=p2;
                                   //                                                              }
                                   //                                                              finished=true;
                                   //                                                      }
                              }
                         }
                    }



                    double vecDA[3];
                    vecDA[0]= (A-D)._x;
                    vecDA[1]= (A-D)._y;
                    vecDA[2]= 0.0;

                    double vecDC[3];
                    vecDC[0]= (C-D)._x;
                    vecDC[1]= (C-D)._y;
                    vecDC[2]= (C-D).Norm()*tan(theta);

                    double vecNormal[3];
                    vecNormal[0]= vecDA[1]*vecDC[2] - vecDA[2]*vecDC[1];
                    vecNormal[1]= vecDA[2]*vecDC[0] - vecDA[0]*vecDC[2];
                    vecNormal[2]= vecDA[0]*vecDC[1] - vecDA[1]*vecDC[0];


                    if((C-D).Norm() < 7.4 ) {
                         base= base - (C-D).Norm()*tan(theta) ;
                         //cout<<" room: "<<r->GetCaption()<<endl;
                    }

                    // the equation of the plan is given as: Ax+By+Cz+d=0;
                    // using the Point A:
                    double d = - (vecNormal[0]*A._x+vecNormal[1]*A._y+vecNormal[2]*base);
                    double coef[3];
                    coef[0]= - vecNormal[0] / vecNormal[2];
                    coef[1]= - vecNormal[1] / vecNormal[2];
                    coef[2]= - d / vecNormal[2];
                    sub->SetPlanEquation(coef[0],coef[1],coef[2]);
               }
          }
     }
}


bool NavMesh::IsCircleVisibleFromLine(const Point& center, double radius, const Line& segment)
{

     int nLine=0;

     for(double alpha=0.0; alpha<=2*M_PI; alpha+=0.01) {

          bool isVisible=true;
          bool done=false;

          double x= radius*cos(alpha);
          double y= radius*sin(alpha);
          Point point_on_circle = Point(x,y) + center;
          //test must be done for the two points separately
          Line seg1=Line(segment.GetPoint1(),point_on_circle);
          //Line seg2=Line(segment.GetPoint2(),point_on_circle);

          for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
               Room* r = _building->GetRoom(i);

               //skip the virtual room containing the complete geometry
               if(r->GetCaption()=="outside") continue;

               for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
                    SubRoom* s = r->GetSubRoom(k);
                    const vector<Wall>& walls = s->GetAllWalls();
                    //const vector<Transition*>& transitions = s->GetAllTransitions();

                    for (unsigned w = 0; w < walls.size(); w++) {

                         //if(walls[w]==segment) continue;
                         // dont check if they share a common vertex
                         //if(walls[w].ShareCommonPointWith(segment)) continue;
                         if(walls[w].HasEndPoint(segment.GetPoint1())) continue;

                         if(seg1.IntersectionWith(walls[w])) {
                              //cout<<"X";
                              done=true;
                              isVisible=false;
                              break;
                         }
                    }
                    if(!done)
                         for (map<int, Transition*>::const_iterator itr = _building->GetAllTransitions().begin();
                                   itr != _building->GetAllTransitions().end(); ++itr) {
                              Transition* cross = itr->second;
                              //if(cross->operator ==(segment)) continue;
                              if(cross->IntersectionWith(segment)) {
                                   done=true;
                                   isVisible=false;
                                   break;
                              }
                         }

                    if(done) break;
               }
               if(done) break;
          }

          //one visibility line was found
          if(isVisible==true) {
               nLine++;
               break;
          }
     }

     //the first point failed.
     // check the second one
     if(nLine==0)    return false;

     //if (nLine==1) return true;

     //restart the same procedure with the second point

     for(double alpha=0.0; alpha<=2*M_PI; alpha+=0.01) {

          bool isVisible=true;
          bool done=false;

          double x= radius*cos(alpha);
          double y= radius*sin(alpha);
          Point point_on_circle = Point(x,y) + center;
          //test must be done for the two points separately
          //Line seg1=Line(segment.GetPoint1(),point_on_circle);
          Line seg2=Line(segment.GetPoint2(),point_on_circle);


          for (int i = 0; i < _building->GetNumberOfRooms(); i++) {
               Room* r = _building->GetRoom(i);

               //skip the virtual room containing the complete geometry
               if(r->GetCaption()=="outside") continue;

               for (int k = 0; k < r->GetNumberOfSubRooms(); k++) {
                    SubRoom* s = r->GetSubRoom(k);
                    const vector<Wall>& walls = s->GetAllWalls();

                    for (unsigned w = 0; w < walls.size(); w++) {

                         //if(walls[w]==segment) continue;
                         //if(walls[w].ShareCommonPointWith(segment)) continue;
                         if(walls[w].HasEndPoint(segment.GetPoint2())) continue;

                         if(seg2.IntersectionWith(walls[w])) {
                              //cout<<"X";
                              done=true;
                              isVisible=false;
                              break;
                         }
                    }
                    if(!done)
                         for (map<int, Transition*>::const_iterator itr = _building->GetAllTransitions().begin();
                                   itr != _building->GetAllTransitions().end(); ++itr) {
                              Transition* cross = itr->second;
                              //if(cross->operator ==(segment)) continue;
                              if(cross->IntersectionWith(segment)) {
                                   done=true;
                                   isVisible=false;
                                   break;
                              }
                         }
                    if(done) break;
               }
               if(done) break;
          }

          //one visibility line was found
          if(isVisible==true) {
               nLine++;
               break;
          }
     }

     cout<<"nline: " <<nLine<<endl;
     if(nLine==2) return true;
     else return false;

}


void NavMesh::Test()
{

     Log->Write("INFO:\tValidating the generated mesh");

     for ( int e=0; e< (int)_edges.size(); e++) {
          if(e!=_edges[e]->id) {
               cout<<"Test failed by edge: "<<e<<" != "<<_edges[e]->id<<endl;
               exit(EXIT_FAILURE);
          }
          if(_edges[e]->pNode0==-1) {
               cout<<"edge id: " <<_edges[e]->id<<endl;
               cout<<"Node 0 id: "<< _edges[e]->pNode0<<endl;
               cout<<"Node 1 id: "<< _edges[e]->pNode1<<endl;
               cout<<"test failed"<<endl;
               exit(EXIT_FAILURE);
          }
          if( (_edges[e]->pNode1)==-1) {
               cout<<"edge id: " <<_edges[e]->id<<endl;
               cout<<"Node 0 id: "<< _edges[e]->pNode0<<endl;
               cout<<"Node 1 id: "<< _edges[e]->pNode1<<endl;
               cout<<"test failed"<<endl;
          }
          if( _edges[e]->pNode1==_edges[e]->pNode0) {
               cout<<"edge id: " <<_edges[e]->id<<endl;
               cout<<"Node 0 id: "<< _edges[e]->pNode0<<endl;
               cout<<"Node 1 id: "<< _edges[e]->pNode1<<endl;
               cout<<"test failed"<<endl;
               exit(EXIT_FAILURE);
          }
     }

     for ( int i=0; i<(int)_obst.size(); i++) {
          if(i!=_obst[i]->id) {
               cout<<"Test failed by Obstacle: "<<i<<" != "<<_obst[i]->id<<endl;
               exit(EXIT_FAILURE);
          }
          if((_obst[i]->pNode0)==-1) {
               cout<<"Node 0 id (obst): "<< _obst[i]->pNode0<<" for obstacle"<<endl;
               cout<<"test failed"<<endl;
               exit(EXIT_FAILURE);
          }

     }
     Log->Write("INFO:\t...Done!");
}

const std::vector<NavMesh::JEdge*>& NavMesh::GetEdges() const
{
     return _edges;
}

const std::vector<NavMesh::JNode*>& NavMesh::GetNodes() const
{
     return _nodes;
}

const std::vector<NavMesh::JObstacle*>& NavMesh::GetObst() const
{
     return _obst;
}

const std::vector<NavMesh::JVertex*>& NavMesh::GetVertices() const
{
     return _vertices;
}
